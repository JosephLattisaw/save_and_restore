import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter_hooks/flutter_hooks.dart';
import 'package:controller/tab_widget.dart' as tab_widget;
import 'package:controller/table_widget.dart' as table_widget;
import 'package:controller/colors.dart';
import 'package:controller/lite_rolling_switch.dart';

void main() {
  runApp(MyApp());
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
      home: MyHomePage(title: 'Controller'),
    );
  }
}

class MyHomePage extends HookWidget {
  MyHomePage({Key? key, required this.title}) : super(key: key);

  final String title;

  static const int TOTAL_TABS = 3;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(title),
      ),
      body: Container(
        child: Column(
          children: [
            Flexible(
              flex: 1,
              child: Padding(
                padding: const EdgeInsets.all(8.0),
                child: Row(
                  children: [
                    Container(
                      /*decoration: const BoxDecoration(
                        color: Colors.red,
                      ),*/
                      padding: const EdgeInsets.all(4.0),
                      child: LiteRollingSwitch(
                        textOff: "TCTS SIM",
                        textOn: "SLE",
                        colorOff: Colors.blue.shade900,
                        colorOn: Colors.orange.shade600,
                        iconOff: Icons.title,
                        iconOn: Icons.public,
                      ),
                    ),
                    Container(
                      /*decoration: const BoxDecoration(
                        color: Colors.red,
                      ),*/
                      padding: const EdgeInsets.all(4.0),
                      child: LiteRollingSwitch(
                          textOff: "ADU VDS",
                          textOn: "ADUSIM",
                          colorOff: Colors.blue.shade900,
                          colorOn: Colors.orange.shade600,
                          iconOff: CupertinoIcons.rocket_fill,
                          iconOn: Icons.brightness_auto),
                    ),
                    Container(
                      padding: const EdgeInsets.all(4.0),
                      child: LiteRollingSwitch(
                        textOff: "NO ISIM",
                        textOn: "ISIM",
                        colorOff: Colors.grey.shade800,
                        colorOn: Colors.orange.shade600,
                        iconOn: CupertinoIcons.star_lefthalf_fill,
                      ),
                    ),
                  ],
                ),
              ),
            ),
            Expanded(
              flex: 8,
              child: Row(
                children: [
                  Expanded(
                    child: Center(child: table_widget.TableWidget()),
                  ),
                  Expanded(
                    flex: 6,
                    child: Container(
                      decoration: BoxDecoration(
                        color: scaffoldBackgroundColor,
                      ),
                      child: Padding(
                        padding: const EdgeInsets.all(8.0),
                        child: tab_widget.TabWidget(
                          tabs: List<Widget>.generate(TOTAL_TABS,
                              (index) => Tab(text: "Tab ${index + 1}")),
                          tabWidgets: List<Widget>.generate(
                            TOTAL_TABS,
                            (index) => Center(
                              child: Text(
                                "Tab ${index + 1}",
                              ),
                            ),
                          ),
                        ),
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}

/*Center(
        child: Row(
          children: [
            Padding(
              padding: const EdgeInsets.all(8.0),
              child: tab_widget.TabWidget(
                tabs: List<Widget>.generate(
                    TOTAL_TABS, (index) => Tab(text: "Tab ${index + 1}")),
                tabWidgets: List<Widget>.generate(
                  TOTAL_TABS,
                  (index) => Center(
                    child: Text(
                      "Tab ${index + 1}",
                      style: TextStyle(color: Colors.white),
                    ),
                  ),
                ),
              ),
            ),
          ],
        ),
      ),*/
