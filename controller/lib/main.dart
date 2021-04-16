import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter_hooks/flutter_hooks.dart';
import 'package:controller/tab_widget.dart' as tab_widget;
import 'package:controller/table_widget.dart' as table_widget;
import 'package:controller/colors.dart';
import 'package:controller/lite_rolling_switch.dart';
import 'package:window_size/window_size.dart';
import 'package:xterm/frontend/terminal_view.dart';
import 'package:xterm/xterm.dart';

void main() {
  runApp(MyApp());
  setWindowTitle("STS Controller"); //has to be called after running the app
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

class SaveAndRestoreScreen extends HookWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        elevation: 10.0,
        title: Row(
          children: [
            Text("Save And Restore"),
          ],
        ),
      ),
      body: Container(
        color: Colors.green,
      ),
    );
  }
}

class MyHomePage extends HookWidget {
  MyHomePage({Key? key, required this.title}) : super(key: key);

  final String title;

  static const int TOTAL_TABS = 1;

  void drawerRouter(BuildContext context, Widget widget) {
    Navigator.push(
      context,
      MaterialPageRoute(
        builder: (context) => widget,
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    Terminal terminal = useMemoized(() => Terminal(onInput: (String) {}));

    useEffect(() {
      terminal.write('connecting to terminal');
    }, const []);

    return Scaffold(
      appBar: AppBar(
        elevation: 10.0,
        title: Row(
          children: [
            Text("STS Controller"),
            Image.asset(
              "assets/honeycomb.png",
              fit: BoxFit.scaleDown,
              height: 35,
              width: 35,
            ),
            Expanded(
              child: SizedBox(width: 1),
            ),
            FloatingActionButton.extended(
              onPressed: () => null,
              icon: Icon(Icons.save),
              label: Text("Save & Restore"),
              backgroundColor: scaffoldBackgroundColor,
              elevation: 4.0,
            ),
            SizedBox(
              width: 10.0,
              height: AppBar().preferredSize.height * .70,
              child: VerticalDivider(
                color: Colors.black.withOpacity(0.2),
                width: 7.0,
                thickness: 2.0,
              ),
            ),
            FloatingActionButton.extended(
              onPressed: () => null,
              icon: Icon(CupertinoIcons.gamecontroller),
              label: Text("Take Control"),
              backgroundColor: scaffoldBackgroundColor,
              elevation: 4.0,
            ),
          ],
        ),
      ),
      body: Container(
        child: Row(
          children: [
            Expanded(
              flex: 8,
              child: Column(
                children: [
                  Flexible(
                    flex: 1,
                    child: Padding(
                      padding: const EdgeInsets.all(8.0),
                      child: Row(
                        children: [
                          Container(
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
                              iconOff: CupertinoIcons.xmark_circle_fill,
                              iconOn: CupertinoIcons.star_lefthalf_fill,
                            ),
                          ),
                          Expanded(
                            child: SizedBox(width: 1),
                          ),
                          FloatingActionButton.extended(
                            onPressed: () => null,
                            label: Text("Start"),
                            icon: Icon(CupertinoIcons.infinite),
                          ),
                          SizedBox(width: 5.0),
                          FloatingActionButton.extended(
                            onPressed: () => null,
                            label: Text("Stop"),
                            icon: Icon(Icons.stop),
                            backgroundColor: null,
                          ),
                          Expanded(
                            child: SizedBox(width: 1),
                          ),
                          FloatingActionButton(
                            onPressed: () => null,
                            child: Icon(Icons.pause),
                          ),
                          SizedBox(width: 8),
                          FloatingActionButton(
                            onPressed: () => null,
                            child: Icon(Icons.play_arrow),
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
                                    child: Padding(
                                      padding: const EdgeInsets.all(8.0),
                                      child: TerminalView(
                                        terminal: terminal,
                                      ),
                                    ), /*Text(
                                      "Tab ${index + 1}",
                                    ),*/
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
            Expanded(
              flex: 1,
              child: Container(
                decoration: BoxDecoration(
                  image: DecorationImage(
                    image: AssetImage('assets/mirror44.jpg'),
                    fit: BoxFit.cover,
                    colorFilter: ColorFilter.mode(
                      Colors.black.withOpacity(0.7),
                      BlendMode.darken,
                    ),
                  ),
                ),
              ),
            )
          ],
        ),
      ),
    );
  }
}
