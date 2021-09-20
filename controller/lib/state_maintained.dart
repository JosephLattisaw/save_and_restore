import 'dart:isolate';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'dart:ffi' as ffi;

class StateMaintained extends ChangeNotifier {
  int? saveAndRestoreLastConfig;
  int? saveAndRestoreLastSelected;
}
