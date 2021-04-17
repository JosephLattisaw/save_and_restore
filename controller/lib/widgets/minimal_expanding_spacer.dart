import 'package:flutter/material.dart';

class MinimalExpandingSpacer extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Expanded(
      child: SizedBox(
        width: _MINIMAL_SPACER_SIZE,
        height: _MINIMAL_SPACER_SIZE,
      ),
    );
  }

  static const double _MINIMAL_SPACER_SIZE = 1;
}
