library table_widget;

import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter_hooks/flutter_hooks.dart';

class TableWidget extends HookWidget {
  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
        color: Colors.grey,
        boxShadow: [
          BoxShadow(
            blurRadius: _TABLE_BLUR_RADIUS,
            spreadRadius: _TABLE_SPREAD_RADIUS,
            color: _tableBoxShadowColor,
          ),
        ],
      ),
      child: Table(
        border: _tableBorder,
        defaultColumnWidth: IntrinsicColumnWidth(),
        children: List<TableRow>.generate(_tableColumnLabels.length, (i) {
          if (i == _HEADER_ROW) {
            return TableRow(
              children: List<Widget>.generate(
                _tableColumnLabels[i].length,
                (k) => Container(
                  padding: _TABLE_PADDING,
                  color: _headerColor,
                  child: Text(
                    _tableColumnLabels[i][k],
                    style: TextStyle(fontWeight: FontWeight.bold),
                    textAlign: TextAlign.center,
                  ),
                ),
              ),
            );
          } else {
            return TableRow(
              children: List<Widget>.generate(
                _tableColumnLabels[i].length,
                (k) => Container(
                  padding: _TABLE_PADDING,
                  child: Text(_tableColumnLabels[i][k]),
                ),
              ),
            );
          }
        }),
      ),
    );
  }

  static List<List<String>> _tableColumnLabels = [
    ["Application", "Status"],
    ["TCTS SIM", "Status"],
    ["SLE", "Status"],
    ["SIMICS", "Status"],
    ["OTB", "Status"],
    ["ADUSIM", "Status"],
    ["ISIM", "Status"],
    ["IES", "Status"],
    ["DSim Client", "Status"],
    ["TSim Client", "Status"],
  ];

  static Color _headerColor = Colors.blue.shade900;
  static const int _HEADER_ROW = 0;

  static const _TABLE_PADDING = const EdgeInsets.all(8.0);
  static TableBorder _tableBorder = TableBorder.all(width: 0.6);

  static const double _TABLE_BLUR_RADIUS = 2;
  static const double _TABLE_SPREAD_RADIUS = 3;
  static Color _tableBoxShadowColor = Colors.black.withOpacity(0.8);
}
