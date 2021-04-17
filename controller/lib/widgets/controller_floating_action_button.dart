import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';

class ControllerFloatingActionButton extends StatelessWidget {
  ControllerFloatingActionButton({
    Key? key,
    this.label,
    required this.onPressed,
    required this.iconData,
    this.backgroundColor,
    this.tooltip,
    this.elevation,
  }) : super(key: key);

  final Color? backgroundColor;
  final double? elevation;
  final IconData iconData;
  final String? label;
  final void Function()? onPressed;
  final String? tooltip;

  @override
  Widget build(BuildContext context) {
    return (this.label == null)
        ? FloatingActionButton(
            onPressed: onPressed,
            elevation: elevation,
            heroTag: null,
            child: Icon(iconData),
            backgroundColor: backgroundColor,
            tooltip: tooltip,
          )
        : FloatingActionButton.extended(
            onPressed: onPressed,
            label: Text(label ?? ""),
            elevation: elevation,
            heroTag: null,
            icon: Icon(iconData),
            backgroundColor: backgroundColor,
            tooltip: tooltip,
          );
  }
}
