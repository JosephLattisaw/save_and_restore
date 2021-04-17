import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter_hooks/flutter_hooks.dart';

import 'package:window_size/window_size.dart';

import 'package:controller/application_bar.dart';
import 'package:controller/colors.dart';
import 'package:controller/right_panel.dart';
import 'package:controller/left_panel.dart';
import 'package:controller/tab_panel.dart';

void main() {
  runApp(MyApp());
  setWindowTitle(
      "Software Telemetry Simulator (STS) Controller"); //has to be called after running the app
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'controller',
      theme: ThemeData(
        primarySwatch: primarySwatch,
        scaffoldBackgroundColor: scaffoldBackgroundColor,
        textTheme: TextTheme(bodyText2: TextStyle(color: Colors.white)),
      ),
      home: MyHomePage(),
    );
  }
}

class MyHomePage extends StatelessWidget {
  MyHomePage({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: ApplicationBar.appBar(),
      body: Container(
        child: Row(
          children: [
            Expanded(
              flex: _LEFT_AND_MIDDLE_PANEL_FLEX,
              child: Column(
                children: [
                  Expanded(
                    child: Row(
                      children: [
                        Expanded(child: LeftPanel()),
                        Expanded(
                          flex: _TAB_PANEL_FLEX,
                          child: TabPanel(),
                        ),
                      ],
                    ),
                  ),
                ],
              ),
            ),
            Expanded(child: RightPanel())
          ],
        ),
      ),
    );
  }

  static const int _LEFT_AND_MIDDLE_PANEL_FLEX = 8;
  static const int _TAB_PANEL_FLEX = 6;
}
