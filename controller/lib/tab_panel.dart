import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';

import 'package:controller/colors.dart';
import 'package:controller/tab_widget.dart' as tab_widget;

class TabPanel extends StatelessWidget {
  static const int TOTAL_TABS = 3;

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
        color: scaffoldBackgroundColor,
      ),
      child: Padding(
        padding: const EdgeInsets.all(8.0),
        child: tab_widget.TabWidget(
          tabs: List<Widget>.generate(
              TOTAL_TABS, (index) => Tab(text: "Tab ${index + 1}")),
          tabWidgets: List<Widget>.generate(
            TOTAL_TABS,
            (index) => Center(
              child: Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text("Tab ${index + 1}"),
              ),
            ),
          ),
        ),
      ),
    );
  }
}
