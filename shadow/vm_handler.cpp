#include "vm_handler.hpp"

#include <iostream>

VMHandler::VMHandler(VMSavedCallback vsav_cb, VMListCallback vlst_cb, VMRunningListCallback vrl_cb, VMSnapsCallback vsnp_cb,
                     boost::asio::io_service &io_service)
    : vm_saved_callback(vsav_cb),
      vm_list_callback(vlst_cb),
      vm_running_list_callback(vrl_cb),
      vm_snaps_callback(vsnp_cb),
      io_service(io_service),
      vm_status_timer(io_service) {
    //     Check that PRUnichar is equal in size to what compiler composes L""
    //     strings from; otherwise NS_LITERAL_STRING macros won't work correctly
    //     and we will get a meaningless SIGSEGV. This, of course, must be checked
    //     at compile time in xpcom/string/nsTDependentString.h, but XPCOM lacks
    //     compile-time assert macros and I'm not going to add them now.

    if (sizeof(PRUnichar) != sizeof(wchar_t)) {
        std::cout << "vm handler: Error: sizeof(PRUnichar) {" << static_cast<unsigned long>(sizeof(PRUnichar)) << "} != sizeof(wchar_t) {"
                  << static_cast<unsigned long>(sizeof(wchar_t)) << "}!" << std::endl;
        std::cout << "vm handler: Probably, you forgot the -fshort-wchar compiler option." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    nsresult rc;

    //
    //     This is the standard XPCOM init procedure.
    //     What we do is just follow the required steps to get an instance
    //     of our main interface, which is IVirtualBox.
    //
    //     Note that we scope all nsCOMPtr variables in order to have all XPCOM
    //     objects automatically released before we call NS_ShutdownXPCOM at the
    //     end. This is an XPCOM requirement.
    //
    rc = NS_InitXPCOM2(getter_AddRefs(serviceManager), nullptr, nullptr);
    if (NS_FAILED(rc)) {
        printf("Error: XPCOM could not be initialized! rc=%#x\n", rc);
        std::cout << "vm handler: Error: XPCOM could not be initialized! rc=" << rc << std::endl;
        std::exit(EXIT_FAILURE);
    }

    //
    //     Make sure the main event queue is created. This event queue is
    //     responsible for dispatching incoming XPCOM IPC messages. The main
    //     thread should run this event queue's loop during lengthy non-XPCOM
    //     operations to ensure messages from the VirtualBox server and other
    //     XPCOM IPC clients are processed. This use case doesn't perform such
    //     operations so it doesn't run the event loop.
    //
    rc = NS_GetMainEventQ(getter_AddRefs(eventQ));
    if (NS_FAILED(rc)) {
        std::cout << "vm handler: Error: could not get main event queue! rc=" << rc << std::endl;
        std::exit(EXIT_FAILURE);
    }

    //
    //     Now XPCOM is ready and we can start to do real work.
    //     IVirtualBox is the root interface of VirtualBox and will be
    //     retrieved from the XPCOM component manager. We use the
    //     XPCOM provided smart pointer nsCOMPtr for all objects because
    //     that's very convenient and removes the need deal with reference
    //     counting and freeing.
    //
    rc = NS_GetComponentManager(getter_AddRefs(manager));
    if (NS_FAILED(rc)) {
        std::cout << "vm handler: Error: could not get component manager! rc=" << rc << std::endl;
        std::exit(EXIT_FAILURE);
    }

    rc = manager->CreateInstanceByContractID(NS_VIRTUALBOX_CONTRACTID, nullptr, NS_GET_IID(IVirtualBox), getter_AddRefs(virtualBox));

    if (NS_FAILED(rc)) {
        std::cout << "vm handler: Error, could not instantiate VirtualBox object! rc=" << rc << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "vm handler: VirtualBox object created" << std::endl;

    rc = manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID, nullptr, NS_GET_IID(ISession), getter_AddRefs(session));

    if (NS_FAILED(rc)) {
        std::cout << "vm handler: Error, could not instantiate Session object! rc=" << rc << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "vm handler: Session object created" << std::endl;

    start_vm_status_timer();
}

VMHandler::~VMHandler() {
    // this is enough to free the IVirtualBox instance -- smart pointers rule!
    virtualBox = nullptr;
    session = nullptr;

    eventQ->ProcessPendingEvents();

    //
    //     Perform the standard XPCOM shutdown procedure.
    //
    NS_ShutdownXPCOM(nullptr);
    std::cout << "vm handler: Done!" << std::endl;
}

void VMHandler::start_vm_status_timer() {
    vm_status_timer.expires_after(std::chrono::seconds(1));
    vm_status_timer.async_wait([&](const boost::system::error_code &error) {
        request_vm_status_update();
        start_vm_status_timer();
    });
}

void VMHandler::request_vm_status_update() {
    vm_list = get_vm_list();
    vm_running_list = get_vm_running_list();

    vm_list_callback(vm_list);
    vm_running_list_callback(vm_running_list);
}

std::vector<std::string> VMHandler::get_vm_list() { return list_vms(virtualBox); }

std::vector<std::uint8_t> VMHandler::get_vm_running_list() { return list_running_vms(virtualBox); }

void VMHandler::get_vm_snaps(std::string vm) {
    vm_snaps.clear();

    std::vector<std::tuple<int, std::string, std::string>> snapshot_list = list_snapshots(virtualBox, vm);
    for (auto i = 0; i < snapshot_list.size(); i++) {
        std::tuple<int, std::string, std::string> snapshot = snapshot_list[i];
        std::string snapshot_name = std::get<1>(snapshot);
        std::string snapshot_desc = std::get<2>(snapshot);
        std::vector<std::string> snap_vector;
        snap_vector.push_back(snapshot_name);
        if (!snapshot_desc.empty()) snap_vector.push_back(snapshot_desc);
        vm_snaps.push_back(snap_vector);
        // TODO we should be sending the tree vs doing this
    }

    vm_snaps_callback(vm, vm_snaps);
}

void VMHandler::restore_vm(std::string vm, std::string restore_name) {
    restore_vm(virtualBox, session, vm, restore_name);
    request_vm_status_update();
}

void VMHandler::save_vm(std::string vm, std::string name, std::string description) {
    save_vm(virtualBox, session, vm, name, description);
    get_vm_snaps(vm);
    request_vm_status_update();
}
void VMHandler::start_vm(std::string vm) {
    start_vm(virtualBox, session, vm);
    request_vm_status_update();
}
void VMHandler::stop_vm(std::string vm) {
    stop_vm(virtualBox, session, vm);
    request_vm_status_update();
}

void VMHandler::delete_snapshot(const nsCOMPtr<IVirtualBox> &pVirtualBox, const nsCOMPtr<ISession> &pSession, std::string vm_name,
                                std::string snapshot_name) {
    nsresult rc;
    nsCOMPtr<IMachine> machine = find_machine(pVirtualBox, vm_name);

    std::cout << "vm handler: attempting to delete vm snapshot" << std::endl;
    if (machine) {
        std::cout << "vm handler: attempting to lock machine" << std::endl;
        rc = machine->LockMachine(pSession, LockType_Shared);
        if (NS_SUCCEEDED(rc)) {
            IMachine *session_machine = nullptr;
            std::cout << "vm handler: attempting to get machine" << std::endl;
            rc = pSession->GetMachine(&session_machine);
            if (NS_SUCCEEDED(rc) && session_machine != nullptr) {
                ISnapshot *snapshot = nullptr;
                nsCString snapshot_name_nsc = nsCString(snapshot_name.c_str());
                PRUnichar *snapshot_name_unichar = ToNewUnicode(snapshot_name_nsc);
                std::cout << "vm handler: attempting to get current snapshot" << std::endl;
                rc = session_machine->FindSnapshot(snapshot_name_unichar, &snapshot);
                if (NS_SUCCEEDED(rc) && snapshot != nullptr) {
                    PRUnichar *id = nullptr;
                    std::cout << "vm handler: attempting to get snapshot id" << std::endl;
                    rc = snapshot->GetId(&id);
                    if (NS_SUCCEEDED(rc) && id != nullptr) {
                        IProgress *progress = nullptr;
                        std::cout << "vm handler: attempting to delete snapshot" << std::endl;
                        rc = session_machine->DeleteSnapshot(id, &progress);
                        if (NS_SUCCEEDED(rc) && progress != nullptr) {
                            std::cout << "vm handler: Deleting snapshot" << snapshot_name << std::endl;
                            rc = show_progress(progress);
                        }
                    }
                }
            }
        }
    }

    pSession->UnlockMachine();
}

nsCOMPtr<IMachine> VMHandler::find_machine(const nsCOMPtr<IVirtualBox> &pVirtualBox, std::string vm_name) {
    nsresult rc;
    nsCOMPtr<IMachine> machine = nullptr;

    IMachine **machines = nullptr;
    PRUint32 machine_cnt = 0;

    std::cout << "vm handler: looking for vm: " << vm_name << std::endl;
    rc = pVirtualBox->GetMachines(&machine_cnt, &machines);
    if (NS_SUCCEEDED(rc)) {
        for (decltype(machine_cnt) i = 0; i < machine_cnt; ++i) {
            IMachine *m = machines[i];
            if (m) {
                std::string current_vm_name = get_vm_name(m);
                if (!current_vm_name.compare(vm_name)) {
                    std::cout << "vm handler: found vm: " << current_vm_name << std::endl;
                    machine = m;
                    break;
                }
            }
        }
    }

    if (machine == nullptr) std::cout << "vm_handler: did not find vm: " << vm_name << std::endl;

    return machine;
}

std::string VMHandler::get_vm_name(nsCOMPtr<IMachine> machine) {
    std::string vm_name;
    PRBool is_accessible = PR_FALSE;
    machine->GetAccessible(&is_accessible);

    if (is_accessible) {
        nsXPIDLString machine_name;
        machine->GetName(getter_Copies(machine_name));
        char *machine_name_ascii = ToNewCString(machine_name);
        vm_name = std::string(machine_name_ascii);
        free(machine_name_ascii);
    } else {
        vm_name = "<inaccessible>";
    }

    return vm_name;
}

std::vector<std::uint8_t> VMHandler::list_running_vms(const nsCOMPtr<IVirtualBox> &pVirtualBox) {
    nsresult rc;
    std::vector<std::uint8_t> vm_running_list;

    IMachine **machines = nullptr;
    PRUint32 machine_cnt = 0;
    rc = pVirtualBox->GetMachines(&machine_cnt, &machines);

    MachineState_T *states = nullptr;
    PRUint32 states_cnt = 0;
    if (NS_SUCCEEDED(rc)) {
        rc = pVirtualBox->GetMachineStates(machine_cnt, machines, &states_cnt, &states);
    }
    if (NS_SUCCEEDED(rc)) {
        for (decltype(machine_cnt) i = 0; (i < machine_cnt) && (i < states_cnt); ++i) {
            if (machines[i]) {
                MachineState_T machine_state = states[i];
                switch (machine_state) {
                    case MachineState_Running:
                    case MachineState_Teleporting:
                    case MachineState_LiveSnapshotting:
                    case MachineState_Paused:
                    case MachineState_TeleportingPausedVM:
                        vm_running_list.push_back(true);
                        break;
                    default:
                        vm_running_list.push_back(false);
                }
            }
        }
    }

    return vm_running_list;
}

std::vector<std::tuple<int, std::string, std::string>> VMHandler::list_snapshots(const nsCOMPtr<IVirtualBox> &pVirtualBox, std::string vm_name) {
    nsresult rc;
    std::vector<std::tuple<int, std::string, std::string>> snapshot_list;
    nsCOMPtr<IMachine> machine = find_machine(pVirtualBox, vm_name);

    if (machine) {
        ISnapshot *pSnapshot = nullptr;
        rc = machine->FindSnapshot(NS_LITERAL_STRING("").get(), &pSnapshot);
        if (NS_FAILED(rc)) {
            std::cout << "This machine does not have any snapshots" << std::endl;
        } else
            std::cout << "found snapshot(s) for VM:" << vm_name << std::endl;

        if (pSnapshot) {
            ISnapshot *pCurrentSnapshot = nullptr;
            rc = machine->GetCurrentSnapshot(&pCurrentSnapshot);
            snapshot_list = show_snapshots(pSnapshot, pCurrentSnapshot, 0);
        }
    }

    return snapshot_list;
}

std::vector<std::string> VMHandler::list_vms(const nsCOMPtr<IVirtualBox> &pVirtualBox) {
    nsresult rc;
    std::vector<std::string> vm_list;

    IMachine **machines = nullptr;
    PRUint32 machine_cnt = 0;

    rc = pVirtualBox->GetMachines(&machine_cnt, &machines);
    if (NS_SUCCEEDED(rc)) {
        for (decltype(machine_cnt) i = 0; i < machine_cnt; ++i) {
            IMachine *machine = machines[i];
            if (machine) {
                vm_list.push_back(get_vm_name(machine));
            }
        }
    }

    return vm_list;
}

void VMHandler::restore_vm(const nsCOMPtr<IVirtualBox> &pVirtualBox, const nsCOMPtr<ISession> &pSession, std::string vm_name,
                           std::string snapshot_name) {
    nsresult rc;
    nsCOMPtr<IMachine> machine = find_machine(pVirtualBox, vm_name);

    std::cout << "attempting to restore vm" << std::endl;
    if (machine) {
        std::cout << "attempting to lock machine" << std::endl;
        rc = machine->LockMachine(pSession, LockType_Shared);
        if (NS_SUCCEEDED(rc)) {
            IMachine *session_machine = nullptr;
            std::cout << "attempting to get machine" << std::endl;
            rc = pSession->GetMachine(&session_machine);
            if (NS_SUCCEEDED(rc) && session_machine != nullptr) {
                ISnapshot *snapshot = nullptr;
                nsCString snapshot_name_nsc = nsCString(snapshot_name.c_str());
                PRUnichar *snapshot_name_unichar = ToNewUnicode(snapshot_name_nsc);
                std::cout << "attempting to get current snapshot" << std::endl;
                rc = session_machine->FindSnapshot(snapshot_name_unichar, &snapshot);
                free(snapshot_name_unichar);
                if (NS_SUCCEEDED(rc) && snapshot != nullptr) {
                    IProgress *progress = nullptr;
                    std::cout << "attempting to restore snapshot" << std::endl;
                    rc = session_machine->RestoreSnapshot(snapshot, &progress);
                    if (NS_SUCCEEDED(rc)) {
                        std::cout << "waiting for completion" << std::endl;
                        show_progress(progress);
                    } else
                        std::cout << "failed to restore" << std::endl;
                } else
                    std::cout << "failed to get current snapshot" << std::endl;
            } else
                std::cout << "failed to get session machine" << std::endl;
        } else
            std::cout << "failed to lock machine" << std::endl;
    } else
        std::cout << "failed to find machine" << std::endl;
    pSession->UnlockMachine();

    start_vm(pVirtualBox, pSession, vm_name);
}

void VMHandler::save_vm(const nsCOMPtr<IVirtualBox> &pVirtualBox, const nsCOMPtr<ISession> &pSession, std::string vm_name, std::string snapshot_name,
                        std::string snapshot_desc) {
    nsresult rc;
    nsCOMPtr<IMachine> machine = find_machine(pVirtualBox, vm_name);

    std::cout << "attempting to save vm" << std::endl;
    if (machine) {
        std::cout << "attempting to lock machine" << std::endl;
        rc = machine->LockMachine(pSession, LockType_Shared);
        if (NS_SUCCEEDED(rc)) {
            IMachine *session_machine = nullptr;
            std::cout << "attempting to get machine" << std::endl;
            rc = pSession->GetMachine(&session_machine);
            if (NS_SUCCEEDED(rc) && session_machine != nullptr) {
                nsCString snapshot_name_nsc = nsCString(snapshot_name.c_str());
                PRUnichar *snapshot_name_unichar = ToNewUnicode(snapshot_name_nsc);
                nsCString snapshot_desc_nsc = nsCString(snapshot_desc.c_str());
                PRUnichar *snapshot_desc_unichar = ToNewUnicode(snapshot_desc_nsc);
                PRBool fPause = true;  // default is NO live snapshot
                PRUnichar *snap_id = nullptr;
                IProgress *progress = nullptr;
                rc = session_machine->TakeSnapshot(snapshot_name_unichar, snapshot_desc_unichar, fPause, &snap_id, &progress);
                free(snapshot_name_unichar);
                free(snapshot_desc_unichar);
                if (NS_SUCCEEDED(rc) && progress != nullptr) {
                    rc = show_progress(progress);
                }
            }
        }
    }

    pSession->UnlockMachine();
}

nsresult VMHandler::show_progress(nsCOMPtr<IProgress> progress) {
    nsresult rc;
    PRUint32 ulCurrentPercent = 0;
    PRUint32 ulLastPercent = 0;
    nsCOMPtr<nsIEventQueue> event_queue;
    rc = NS_GetMainEventQ(getter_AddRefs(event_queue));

    if (NS_SUCCEEDED(rc)) {
        event_queue->ProcessPendingEvents();
    } else {
        std::cout << "failed to process pending events" << std::endl;
        return rc;
    }

    PRUint32 cOperations = 1;
    rc = progress->GetOperationCount(&cOperations);
    if (NS_FAILED(rc)) {
        std::cout << "Progress object failure " << rc << std::endl;
        return rc;
    }

    std::cout << "0 %" << std::endl;
    PRBool fCancelable;
    rc = progress->GetCancelable(&fCancelable);
    if (NS_FAILED(rc)) {
        fCancelable = false;
    }
    if (fCancelable) {
        // TODO
    }

    PRBool fCompleted;
    rc = progress->GetCompleted(&fCompleted);
    while (NS_SUCCEEDED(rc)) {
        rc = progress->GetPercent(&ulCurrentPercent);
        if (NS_FAILED(rc)) break;

        if (ulCurrentPercent / 10 > ulLastPercent / 10) {
            // make sure to also print out missed steps
            for (unsigned long curVal = (ulLastPercent / 10) * 10 + 10; curVal <= (ulCurrentPercent / 10) * 10; curVal += 10) {
                if (curVal < 100) {
                    std::cout << curVal << " %" << std::endl;
                }
            }
            ulLastPercent = (ulCurrentPercent / 10) * 10;
        }

        if (fCompleted) break;

        // make sure the loop is not too tight
        progress->WaitForCompletion(100);

        event_queue->ProcessPendingEvents();
        rc = progress->GetCompleted(&fCompleted);
    }

    PRInt32 irc;
    rc = progress->GetResultCode(&irc);
    if (NS_SUCCEEDED(rc)) {
        if (NS_SUCCEEDED(irc)) {
            std::cout << 100 << " %" << std::endl;
        } else
            std::cout << "FAILED" << std::endl;
    } else
        std::cout << "progress object failure" << std::endl;

    return rc;
}

std::vector<std::tuple<int, std::string, std::string>> VMHandler::show_snapshots(nsCOMPtr<ISnapshot> rootSnapshot,
                                                                                 nsCOMPtr<ISnapshot> currentSnapshot, int level) {
    nsresult rc;
    std::vector<std::tuple<int, std::string, std::string>> snapshot_list;
    std::string current_snapshot_printout = "";
    std::string snapshot_prefix_printout = "";
    std::string snapshot_indention_printout = "   ";

    for (auto i = 0; i < level; i++) snapshot_prefix_printout.insert(0, snapshot_indention_printout);

    if (rootSnapshot == currentSnapshot) current_snapshot_printout = "(current_snapshot)";

    // start with the root
    nsXPIDLString snapshot_name;
    nsXPIDLString snapshot_description;
    std::string snapshot_name_qstring;
    std::string snapshot_description_qstring;

    rc = rootSnapshot->GetName(getter_Copies(snapshot_name));
    rc = rootSnapshot->GetDescription(getter_Copies(snapshot_description));
    char *snapshot_name_ascii = ToNewCString(snapshot_name);
    char *snapshot_description_ascii = ToNewCString(snapshot_description);
    snapshot_name_qstring = std::string(snapshot_name_ascii);
    snapshot_description_qstring = std::string(snapshot_description_ascii);
    free(snapshot_name_ascii);
    free(snapshot_description_ascii);

    snapshot_list.push_back(std::make_tuple(level, snapshot_name_qstring, snapshot_description_qstring));
    std::cout << snapshot_prefix_printout.c_str() << "snapshot:" << snapshot_name_qstring << "description:" << snapshot_description_qstring
              << current_snapshot_printout << std::endl;

    // get the children
    ISnapshot **coll = nullptr;
    PRUint32 coll_cnt = 0;
    rc = rootSnapshot->GetChildren(&coll_cnt, &coll);
    if (NS_SUCCEEDED(rc) && coll != nullptr) {
        for (decltype(coll_cnt) index = 0; index < coll_cnt; ++index) {
            ISnapshot *snapshot = coll[index];
            if (snapshot) {
                // recursive call
                std::vector<std::tuple<int, std::string, std::string>> s_list = show_snapshots(snapshot, currentSnapshot, level + 1);
                for (auto i = 0; i < s_list.size(); i++) {
                    snapshot_list.push_back(s_list.at(i));
                }
            }
        }
    }

    return snapshot_list;
}

void VMHandler::start_vm(const nsCOMPtr<IVirtualBox> &pVirtualBox, const nsCOMPtr<ISession> &pSession, std::string vm_name) {
    nsresult rc;
    nsCOMPtr<IMachine> machine = find_machine(pVirtualBox, vm_name);

    if (machine) {
        IProgress *progress = nullptr;
        rc = machine->LaunchVMProcess(pSession, NS_LITERAL_STRING("gui").get(), 0, NULL, &progress);
        if (NS_SUCCEEDED(rc) && progress != nullptr) {
            std::cout << "Waiting for VM" << vm_name << "to power on" << std::endl;
            show_progress(progress);
            rc = progress->WaitForCompletion(-1);
            if (NS_SUCCEEDED(rc)) {
                std::cout << "VM" << vm_name << "has been successfully started" << std::endl;
            }
        }
    }

    pSession->UnlockMachine();
}

void VMHandler::stop_vm(const nsCOMPtr<IVirtualBox> &pVirtualBox, const nsCOMPtr<ISession> &pSession, std::string vm_name) {
    nsresult rc;
    nsCOMPtr<IMachine> machine = find_machine(pVirtualBox, vm_name);

    if (machine) {
        rc = machine->LockMachine(pSession, LockType_Shared);
        if (NS_SUCCEEDED(rc)) {
            IConsole *console = nullptr;
            rc = pSession->GetConsole(&console);
            if (NS_SUCCEEDED(rc) && console != nullptr) {
                IProgress *progress = nullptr;
                rc = console->PowerDown(&progress);
                rc = show_progress(progress);
            }
        }
    }

    pSession->UnlockMachine();
}