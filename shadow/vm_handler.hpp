#ifndef VM_HANDLER_HPP
#define VM_HANDLER_HPP

#include <functional>
#include <string>
#include <vector>

// XPCOM headers
#include <nsEventQueueUtils.h>
#include <nsIExceptionService.h>
#include <nsIServiceManager.h>
#include <nsMemory.h>
#include <nsString.h>

// XPCOM Interface
#include "VirtualBox_XPCOM.h"

class VMHandler {
public:
    using VMSavedCallback = std::function<void()>;
    using VMListCallback = std::function<void(std::vector<std::string>)>;
    using VMRunningListCallback = std::function<void(std::vector<bool>)>;
    using VMSnapsCallback = std::function<void(std::string, std::vector<std::vector<std::string>>)>;

    VMHandler(VMSavedCallback vm_saved_callback, VMListCallback vm_list_callback, VMRunningListCallback vm_running_list_callback,
              VMSnapsCallback vm_snaps_callback);
    ~VMHandler();

    void get_vm_snaps(std::string vm);
    void request_vm_status_update();
    void restore_vm(std::string vm, std::string restore_name);
    void save_vm(std::string vm, std::string name, std::string description);
    void start_vm(std::string vm);
    void stop_vm(std::string vm);

private:
    std::vector<std::string> get_vm_list();
    std::vector<bool> get_vm_running_list();

    void delete_snapshot(const nsCOMPtr<IVirtualBox> &pVirtualBox, const nsCOMPtr<ISession> &pSession, std::string vm_name,
                         std::string snapshot_name);
    nsCOMPtr<IMachine> find_machine(const nsCOMPtr<IVirtualBox> &pVirtualBox, std::string vm_name);
    std::string get_vm_name(nsCOMPtr<IMachine> machine);
    std::vector<bool> list_running_vms(const nsCOMPtr<IVirtualBox> &pVirtualBox);
    std::vector<std::tuple<int, std::string, std::string>> list_snapshots(const nsCOMPtr<IVirtualBox> &pVirtualBox, std::string vm_name);
    std::vector<std::string> list_vms(const nsCOMPtr<IVirtualBox> &pVirtualBox);
    void restore_vm(const nsCOMPtr<IVirtualBox> &pVirtualBox, const nsCOMPtr<ISession> &pSession, std::string vm_name, std::string snapshot_name);
    void save_vm(const nsCOMPtr<IVirtualBox> &pVirtualBox, const nsCOMPtr<ISession> &pSession, std::string vm_name, std::string snapshot_name,
                 std::string snapshot_desc);
    nsresult show_progress(nsCOMPtr<IProgress> progress);
    std::vector<std::tuple<int, std::string, std::string>> show_snapshots(nsCOMPtr<ISnapshot> rootSnapshot, nsCOMPtr<ISnapshot> currentSnapshot,
                                                                          int level);
    void start_vm(const nsCOMPtr<IVirtualBox> &pVirtualBox, const nsCOMPtr<ISession> &pSession, std::string vm_name);
    void stop_vm(const nsCOMPtr<IVirtualBox> &pVirtualBox, const nsCOMPtr<ISession> &pSession, std::string vm_name);

    nsCOMPtr<nsIEventQueue> eventQ;
    nsCOMPtr<nsIComponentManager> manager;
    nsCOMPtr<nsIServiceManager> serviceManager;
    nsCOMPtr<ISession> session;
    nsCOMPtr<IVirtualBox> virtualBox;

    std::vector<std::string> vm_list;
    std::vector<bool> vm_running_list;
    std::vector<std::vector<std::string>> vm_snaps;

    VMSavedCallback vm_saved_callback;
    VMListCallback vm_list_callback;
    VMRunningListCallback vm_running_list_callback;
    VMSnapsCallback vm_snaps_callback;
};

#endif