import 'package:flutter/foundation.dart';

class Server extends ChangeNotifier {
  Server() {
    print("Server got this\n");
  }

  void takeControlOfSatellite() {
    controlOfSatellite = true;
    notifyListeners();
  }

  bool controlOfSatellite = false;
}
