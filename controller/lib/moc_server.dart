import 'package:flutter/foundation.dart';

class Server extends ChangeNotifier {
  Server() {
    print("Server got this\n");
  }

  void startSimics(bool flag) {
    simicsPlaying = flag;
    notifyListeners();
  }

  void startSimulation() {
    simulationStarted = true;
    notifyListeners();
  }

  void stopSimulation() {
    simulationStarted = false;
    notifyListeners();
  }

  void takeControlOfSatellite() {
    controlOfSatellite = true;
    notifyListeners();
  }

  List<List<dynamic>> applications = [
    ["Applications", "Status"],
    ["TCTS SIM", NOT_RUNNING],
    ["SLE", RUNNING],
    ["Simics", RUNNING],
    ["OTB", RUNNING],
    ["ADUSIM", NOT_RUNNING],
    ["ISIM", CRASHED],
    ["IES", CRASHED],
    ["DSim Client", NOT_RUNNING],
    ["TSim Client", NOT_RUNNING],
  ];

  static const List<String> statusType = [
    "NOT_RUNNING",
    "RUNNING",
    "CRASHED",
  ];

  static const int NOT_RUNNING = 0;
  static const int RUNNING = 1;
  static const CRASHED = 2;

  bool controlOfSatellite = false;
  bool simulationStarted = false;
  bool simicsPlaying = false;
}
