import 'package:flutter/material.dart';

class RouterUtility {
  static void routerUtility(BuildContext context, Widget widget) {
    Navigator.push(
      context,
      MaterialPageRoute(builder: (context) => widget),
    );
  }
}
