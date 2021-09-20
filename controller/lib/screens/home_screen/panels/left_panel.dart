import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';

import 'package:provider/provider.dart';

import 'package:controller/screens/home_screen/configuration_toggle_buttons.dart';
import 'package:controller/widgets/minimal_expanding_spacer.dart';
import 'package:controller/screens/home_screen/table_widget.dart'
    as table_widget;
import 'package:controller/common/colors.dart';
import 'package:controller/common/router_utility.dart';
import 'package:controller/screens/save_and_restore_screen.dart';
import 'package:controller/widgets/controller_floating_action_button.dart';
import 'package:controller/moc_server.dart';
import 'package:controller/shadow_client_c_api.dart';

class LeftPanel extends StatelessWidget {
  bool saveAndRestoreActive(BuildContext context) {
    final c_api = Provider.of<ShadowClientCAPI>(context, listen: false);

    return c_api.controlOfSatellite &&
        ((c_api.vmRunning && c_api.simulationStarted) ||
            !c_api.simulationStarted);
  }

  @override
  Widget build(BuildContext context) {
    final controlOfSatellite =
        context.select((ShadowClientCAPI s) => s.controlOfSatellite);
    final serverConnected =
        context.select((ShadowClientCAPI s) => s.serverConnected);
    final vmRunning = context.select((ShadowClientCAPI s) => s.vmRunning);
    final simulationStarted =
        context.select((ShadowClientCAPI s) => s.simulationStarted);

    final loh = controlOfSatellite &&
        ((vmRunning && simulationStarted) || !simulationStarted);

    print("lp: control: $controlOfSatellite");
    print("lp: vmrunning: $vmRunning");
    print("lp: sim started: $simulationStarted");
    print("lp: logic: ${loh}");

    return Padding(
      padding: _DEFAULT_PADDING,
      child: Column(
        children: [
          ConfigurationToggleButtons(),
          MinimalExpandingSpacer(),
          table_widget.TableWidget(),
          MinimalExpandingSpacer(),
          ControllerFloatingActionButton(
            onPressed: saveAndRestoreActive(context)
                ? () =>
                    RouterUtility.routerUtility(context, SaveAndRestoreScreen())
                : null,
            iconData: _SAVE_AND_RESTORE_ICON,
            label: _SAVE_AND_RESTORE_LABEL,
            backgroundColor: saveAndRestoreActive(context)
                ? primarySwatch
                : Colors.grey.shade900,
            elevation: _SAVE_AND_RESTORE_ELEVATION,
          ),
          SizedBox(height: _DEFAULT_SPACER_HEIGHT),
          Row(
            children: [
              Expanded(
                child: Container(
                  color: serverConnected ? Colors.green : _disconnectedColor,
                  child: Text(
                    serverConnected
                        ? "Connected to Server"
                        : _DISCONNECTED_TEXT,
                    textAlign: _CONNECTION_STATUS_ALIGNMENT,
                  ),
                ),
              ),
            ],
          )
        ],
      ),
    );
  }

  static const EdgeInsetsGeometry _DEFAULT_PADDING = const EdgeInsets.only(
    bottom: 8.0,
    top: 14.0,
  );

  static const double _DEFAULT_SPACER_HEIGHT = 16;

  //connection status items
  static Color _disconnectedColor = Colors.red;
  static const String _DISCONNECTED_TEXT = "Disconnected from Server";
  static const TextAlign _CONNECTION_STATUS_ALIGNMENT = TextAlign.center;

  //save & restore items
  static const double _SAVE_AND_RESTORE_ELEVATION = 4;
  static const IconData _SAVE_AND_RESTORE_ICON = Icons.save;
  static const String _SAVE_AND_RESTORE_LABEL = "Save & Restore";
}
