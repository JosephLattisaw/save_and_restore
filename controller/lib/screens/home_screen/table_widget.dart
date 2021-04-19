library table_widget;

import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter_hooks/flutter_hooks.dart';
import 'package:provider/provider.dart';

import 'package:controller/moc_server.dart';

class TableWidget extends HookWidget {
  @override
  Widget build(BuildContext context) {
    final server = Provider.of<Server>(context, listen: false);
    final appStatuses = context.select((Server s) => s.applications);

    Color? _getTableColor(int row, int col) {
      if (row > 0) {
        final server = Provider.of<Server>(context, listen: false);
        if (server.applications[row][col] == Server.RUNNING) {
          return Colors.green;
        } else if (server.applications[row][col] == Server.CRASHED) {
          return Colors.red;
        }
      }

      return null;
    }

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
        children: List<TableRow>.generate(appStatuses.length, (i) {
          if (i == _HEADER_ROW) {
            return TableRow(
              children: List<Widget>.generate(
                appStatuses[i].length,
                (k) => Container(
                  padding: _TABLE_PADDING,
                  color: _headerColor,
                  child: Text(
                    appStatuses[i][k],
                    style: TextStyle(fontWeight: FontWeight.bold),
                    textAlign: TextAlign.center,
                  ),
                ),
              ),
            );
          } else {
            return TableRow(
              children: List<Widget>.generate(
                appStatuses[i].length,
                (k) => Container(
                  padding: _TABLE_PADDING,
                  child: Text(
                    (k == 0)
                        ? appStatuses[i][k]
                        : Server.statusType[appStatuses[i][k]],
                    textAlign: TextAlign.center,
                  ),
                  color: _getTableColor(i, k),
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
