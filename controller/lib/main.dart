import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter_hooks/flutter_hooks.dart';

import 'package:window_size/window_size.dart';
import 'package:provider/provider.dart';

import 'package:controller/common/colors.dart';
import 'package:controller/screens/home_screen/home_screen.dart';
import 'package:controller/moc_server.dart';

void main() {
  runApp(MyApp());
  setWindowTitle(
      "Software Telemetry Simulator (STS) Controller"); //has to be called after running the app
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MultiProvider(
      providers: [
        ChangeNotifierProvider(
          create: (_) => Server(),
          lazy: false,
        ),
      ],
      child: MaterialApp(
        title: 'controller',
        theme: ThemeData(
          primarySwatch: primarySwatch,
          scaffoldBackgroundColor: scaffoldBackgroundColor,
          textTheme: TextTheme(bodyText2: TextStyle(color: Colors.white)),
        ),
        home: HomeScreen(),
      ),
    );
  }
}
