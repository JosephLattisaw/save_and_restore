import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_hooks/flutter_hooks.dart';
import 'package:controller/common/colors.dart';
import 'package:controller/shadow_client_c_api.dart';
import 'package:provider/provider.dart';
import 'package:controller/state_maintained.dart';
import 'package:controller/screens/home_screen/application_bar.dart';

class SaveAndRestoreScreen extends HookWidget {
  bool unSaveableState(BuildContext context) {
    final server = Provider.of<ShadowClientCAPI>(context, listen: false);
    return (server.simicsPlaying || !server.vmRunning);
  }

  @override
  Widget build(BuildContext context) {
    final vmList = context.select((ShadowClientCAPI s) => s.vmList);
    final vmRunningList =
        context.select((ShadowClientCAPI s) => s.vmRunningList);

    final chosenValueInitial =
        Provider.of<StateMaintained>(context, listen: false);

    final client = Provider.of<ShadowClientCAPI>(context, listen: false);

    final chosenValue =
        useState<int?>(chosenValueInitial.saveAndRestoreLastConfig);
    final selectedRow = useState<int?>(null);

    final serverConnected =
        context.select((ShadowClientCAPI s) => s.serverConnected);
    final controlOfSatellite =
        context.select((ShadowClientCAPI s) => s.controlOfSatellite);
    final simulationStarted =
        context.select((ShadowClientCAPI s) => s.simulationStarted);
    final vmRunning = context.select((ShadowClientCAPI s) => s.vmRunning);
    final simicsPlaying =
        context.select((ShadowClientCAPI s) => s.simicsPlaying);

    final saveFilenameTextController =
        useMemoized(() => TextEditingController());
    final descFilenameTextController =
        useMemoized(() => TextEditingController());

    if (!serverConnected ||
        !controlOfSatellite ||
        (simulationStarted && !vmRunning)) {
      print("sr: popping navigator");
      Navigator.of(context).maybePop();
    }

    if (unSaveableState(context)) {
      saveFilenameTextController.clear();
      descFilenameTextController.clear();
    }

    //sanity check
    if ((chosenValue.value ?? 0) > vmList.length ||
        vmList.length != vmRunningList?.length) {
      chosenValue.value = null;
    }

    //saving values
    chosenValueInitial.saveAndRestoreLastConfig = chosenValue.value;

    return Scaffold(
      extendBody: true,
      appBar: PreferredSize(
        preferredSize: AppBar().preferredSize,
        child: ApplicationBar(
          title: _SAVE_AND_RESTORE_TITLE_LABEL,
          showIcon: false,
          showStartStop: false,
        ),
      ),
      body: Container(
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Container(
              child: Padding(
                padding: const EdgeInsets.only(
                    left: 16.0, right: 16.0, top: 8.0, bottom: 24.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    Text("Machines"),
                    SizedBox(height: 8.0),
                    Container(
                      decoration: BoxDecoration(
                        color: scaffoldBackgroundColor,
                        boxShadow: [
                          BoxShadow(
                            blurRadius: 5,
                            spreadRadius: 4,
                            color: Colors.black.withOpacity(0.8),
                            offset: Offset(0, 10),
                          )
                        ],
                      ),
                      child: Padding(
                        padding: const EdgeInsets.all(8.0),
                        child: Container(
                          child: Row(
                            children: [
                              Text("Configuration:"),
                              SizedBox(width: 8.0),
                              Flexible(
                                child: Container(
                                  color: Colors.blue.shade900,
                                  child: DropdownButtonHideUnderline(
                                    child: DropdownButton(
                                      dropdownColor: Colors.blue.shade900,
                                      value: chosenValue.value,
                                      isDense: true,
                                      isExpanded: true,
                                      style: TextStyle(
                                        fontSize: 14.0,
                                        color: Colors.white,
                                      ),
                                      items:
                                          List<DropdownMenuItem<int>>.generate(
                                        vmList.length,
                                        (index) => DropdownMenuItem(
                                          child: Padding(
                                            padding: const EdgeInsets.only(
                                                left: 8.0),
                                            child: Text(
                                              "${vmList[index]}",
                                              style: TextStyle(
                                                  color: Colors.white),
                                            ),
                                          ),
                                          value: index,
                                        ),
                                      ),
                                      hint: Text(
                                        "Please Choose a Machine Configuration",
                                        style: TextStyle(color: Colors.white),
                                      ),
                                      onChanged: (int? value) {
                                        chosenValue.value = value;
                                      },
                                    ),
                                  ),
                                ),
                              ),
                              Expanded(child: SizedBox(width: 1)),
                              Text("Machine Status:"),
                              SizedBox(width: 8.0),
                              Flexible(
                                child: Theme(
                                  data: ThemeData(
                                      disabledColor:
                                          Colors.black.withOpacity(0.8)),
                                  child: TextFormField(
                                    enabled: false,
                                    style: TextStyle(
                                      fontSize: 14.0,
                                    ),
                                    decoration: InputDecoration(
                                      isDense: true,
                                      fillColor: vmRunningList?.elementAt(
                                                  chosenValue.value ?? 0) ==
                                              0
                                          ? Colors.red
                                          : Colors.green,
                                      filled: true,
                                      hintText: vmRunningList?.elementAt(
                                                  chosenValue.value ?? 0) ==
                                              0
                                          ? "NOT RUNNING"
                                          : "RUNNING",
                                      contentPadding: EdgeInsets.all(8.0),
                                      border: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(0.0)),
                                      ),
                                    ),
                                  ),
                                ),
                              ),
                            ],
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ),
            Container(
              child: Padding(
                padding: const EdgeInsets.only(
                  left: 16.0,
                  right: 16.0,
                ),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    Text(
                      "Save Configuration",
                      style: TextStyle(
                          color: simicsPlaying ? Colors.white : Colors.grey),
                    ),
                    SizedBox(height: 8.0),
                    Container(
                      decoration: BoxDecoration(
                        color: scaffoldBackgroundColor,
                        boxShadow: [
                          BoxShadow(
                            blurRadius: 5,
                            spreadRadius: 4,
                            color: Colors.black.withOpacity(0.8),
                            offset: Offset(0, 10),
                          )
                        ],
                      ),
                      child: Padding(
                        padding: const EdgeInsets.all(8.0),
                        child: Row(
                          children: [
                            Text(
                              "Name:",
                              style: TextStyle(
                                color:
                                    simicsPlaying ? Colors.white : Colors.grey,
                              ),
                            ),
                            SizedBox(width: 8.0),
                            Flexible(
                              child: TextFormField(
                                enabled: !unSaveableState(context),
                                controller: saveFilenameTextController,
                                inputFormatters: [
                                  //max filename length linux
                                  LengthLimitingTextInputFormatter(255),
                                ],
                                style: TextStyle(
                                  color: Colors.black,
                                  fontSize: 14.0,
                                ),
                                decoration: InputDecoration(
                                  isDense: true,
                                  fillColor: Colors.white,
                                  filled: true,
                                  hintText: "Saved Configuration File Name",
                                  contentPadding: EdgeInsets.all(8.0),
                                  border: OutlineInputBorder(
                                    borderRadius:
                                        BorderRadius.all(Radius.circular(0.0)),
                                  ),
                                ),
                              ),
                            ),
                            Expanded(child: SizedBox(width: 1.0)),
                            Text(
                              "Description:",
                              style: TextStyle(
                                color:
                                    simicsPlaying ? Colors.white : Colors.grey,
                              ),
                            ),
                            SizedBox(width: 8.0),
                            Flexible(
                              child: TextFormField(
                                controller: descFilenameTextController,
                                enabled: !unSaveableState(context),
                                style: TextStyle(
                                  color: Colors.black,
                                  fontSize: 14.0,
                                ),
                                decoration: InputDecoration(
                                  isDense: true,
                                  fillColor: Colors.white,
                                  filled: true,
                                  hintText: "Description of Saved Snapshot",
                                  contentPadding: EdgeInsets.all(8.0),
                                  border: OutlineInputBorder(
                                    borderRadius:
                                        BorderRadius.all(Radius.circular(0.0)),
                                  ),
                                ),
                              ),
                            ),
                          ],
                        ),
                      ),
                    ),
                    SizedBox(height: 8.0),
                    Row(
                      mainAxisAlignment: MainAxisAlignment.end,
                      children: [
                        ElevatedButton(onPressed: () {}, child: Text("Save")),
                      ],
                    ),
                  ],
                ),
              ),
            ),
            Expanded(
              child: Padding(
                padding: const EdgeInsets.only(
                  left: 16.0,
                  right: 16.0,
                  bottom: 8.0,
                ),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    Text("Restore Configuration"),
                    SizedBox(height: 8.0),
                    Expanded(
                      child: Container(
                        decoration: BoxDecoration(
                          color: scaffoldBackgroundColor,
                          boxShadow: [
                            BoxShadow(
                              blurRadius: 5,
                              spreadRadius: 4,
                              color: Colors.black.withOpacity(0.8),
                              offset: Offset(0, 10),
                            )
                          ],
                        ),
                        child: RawScrollbar(
                          isAlwaysShown: true,
                          thickness: 12.0,
                          thumbColor: primarySwatch,
                          radius: Radius.circular(10),
                          child: SingleChildScrollView(
                            child: Container(
                              child: Theme(
                                data: Theme.of(context).copyWith(
                                  dividerColor: Colors.black.withOpacity(0.8),
                                ),
                                child: DataTable(
                                  showCheckboxColumn: false,
                                  headingRowColor:
                                      MaterialStateProperty.resolveWith<Color>(
                                          (states) => Colors.grey.shade900),
                                  columns: [
                                    DataColumn(
                                      label: Text(
                                        "Name",
                                        style: TextStyle(color: Colors.white),
                                      ),
                                    ),
                                    DataColumn(
                                      label: Text(
                                        "Description",
                                        style: TextStyle(color: Colors.white),
                                      ),
                                    ),
                                  ],
                                  rows: List<DataRow>.generate(30, (index) {
                                    return DataRow(
                                      selected: index == selectedRow.value,
                                      onSelectChanged: (bool? value) {
                                        selectedRow.value =
                                            (value ?? false) ? index : -1;
                                      },
                                      cells: [
                                        DataCell(Text("STS VM")),
                                        DataCell(Text(
                                            "This VM is useful for a ton of purposes")),
                                      ],
                                      color: MaterialStateProperty.resolveWith<
                                          Color>(
                                        (states) => (selectedRow.value == index)
                                            ? Colors.blue.shade900
                                            : scaffoldBackgroundColor,
                                      ),
                                    );
                                  }),
                                ),
                              ),
                            ),
                          ),
                        ),
                      ),
                    ),
                    SizedBox(height: 14.0),
                    Row(
                      mainAxisAlignment: MainAxisAlignment.end,
                      children: [
                        ElevatedButton(
                            onPressed: () {}, child: Text("Restore")),
                      ],
                    ),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  static const String _SAVE_AND_RESTORE_TITLE_LABEL = "Save And Restore";
}
