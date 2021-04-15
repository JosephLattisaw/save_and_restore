import 'package:flutter/material.dart';
import 'package:flutter_hooks/flutter_hooks.dart';
import 'package:controller/tab_widget.dart' as tab_widget;
import 'package:controller/colors.dart';

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
      body: Center(
        child: Padding(
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
      ),
    );
  }
}
