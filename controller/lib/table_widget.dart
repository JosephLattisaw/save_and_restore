library table_widget;

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
            blurRadius: 2,
            spreadRadius: 3,
            color: Colors.black.withOpacity(0.8),
          ),
        ],
      ),
      child: Table(
        border: TableBorder.all(width: 0.6),
        defaultColumnWidth: IntrinsicColumnWidth(),
        children: [
          TableRow(
            children: [
              Container(
                padding: const EdgeInsets.all(8.0),
                color: Colors.blue.shade900,
                child: Text(
                  "Application",
                  style: TextStyle(fontWeight: FontWeight.bold),
                  textAlign: TextAlign.center,
                ),
              ),
              Container(
                padding: const EdgeInsets.all(8.0),
                color: Colors.blue.shade900,
                child: Text(
                  "Status",
                  style: TextStyle(fontWeight: FontWeight.bold),
                  textAlign: TextAlign.center,
                ),
              ),
            ],
          ),
          TableRow(
            children: [
              Container(
                padding: const EdgeInsets.all(8.0),
                child: Text("TCTS SIM"),
              ),
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("RUNNING"),
              ),
            ],
          ),
          TableRow(
            children: [
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("SLE"),
              ),
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("RUNNING"),
              ),
            ],
          ),
          TableRow(
            children: [
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("Simics"),
              ),
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("RUNNING"),
              ),
            ],
          ),
          TableRow(
            children: [
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("OTB"),
              ),
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("RUNNING"),
              ),
            ],
          ),
          TableRow(
            children: [
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("ADUSIM"),
              ),
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("RUNNING"),
              ),
            ],
          ),
          TableRow(
            children: [
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("ISIM"),
              ),
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("RUNNING"),
              ),
            ],
          ),
          TableRow(
            children: [
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("IES"),
              ),
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("RUNNING"),
              ),
            ],
          ),
          TableRow(
            children: [
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("DSim Client"),
              ),
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("RUNNING"),
              ),
            ],
          ),
          TableRow(
            children: [
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("TSim Client"),
              ),
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("RUNNING"),
              ),
            ],
          ),
        ],
      ),
    );
  }
}
