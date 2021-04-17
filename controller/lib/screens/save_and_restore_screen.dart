import 'package:flutter/material.dart';
import 'package:flutter_hooks/flutter_hooks.dart';

class SaveAndRestoreScreen extends HookWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        elevation: 10.0,
        title: Row(children: [Text(_SAVE_AND_RESTORE_TITLE_LABEL)]),
      ),
      body: Container(color: Colors.green),
    );
  }

  static const String _SAVE_AND_RESTORE_TITLE_LABEL = "Save And Restore";
}
