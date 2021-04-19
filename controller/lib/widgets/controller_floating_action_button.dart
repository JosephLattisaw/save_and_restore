import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';

class ControllerFloatingActionButton extends StatelessWidget {
  ControllerFloatingActionButton({
    Key? key,
    this.label,
    required this.onPressed,
    required this.iconData,
    this.backgroundColor,
    this.hoverColor,
    this.tooltip,
    this.elevation,
  }) : super(key: key) {
    print("CFAB");
  }

  final Color? backgroundColor;
  final Color? hoverColor;
  final double? elevation;
  final IconData iconData;
  final String? label;
  final void Function()? onPressed;
  final String? tooltip;
  final bool disabled = true;

  @override
  Widget build(BuildContext context) {
    return (this.label == null)
        ? FloatingActionButton(
            onPressed: onPressed ?? null,
            elevation: elevation,
            heroTag: null,
            child: Icon(iconData),
            backgroundColor: backgroundColor,
            hoverColor: hoverColor,
            tooltip: tooltip,
          )
        : FloatingActionButton.extended(
            onPressed: onPressed ?? null,
            label: Text(label ?? ""),
            elevation: elevation,
            heroTag: null,
            icon: Icon(iconData),
            backgroundColor: backgroundColor,
            hoverColor: hoverColor,
            tooltip: tooltip,
          );
  }
}
