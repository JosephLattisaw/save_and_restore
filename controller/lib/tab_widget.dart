library tab_widget;

import 'package:controller/colors.dart';
import 'package:flutter/material.dart';
import 'package:flutter_hooks/flutter_hooks.dart';

class TabWidget extends HookWidget {
  TabWidget({Key? key, required this.tabs, required this.tabWidgets})
      : super(key: key) {
    assert(this.tabs.length == this.tabWidgets.length); //sanity check
  }

  final List<Widget> tabs;
  final List<Widget> tabWidgets;

  @override
  Widget build(BuildContext context) {
    final ticker = useSingleTickerProvider();
    final tabController =
        useMemoized(() => TabController(length: tabs.length, vsync: ticker));
    final defaultBorderRadius = useMemoized(() => BorderRadius.circular(10.0));

    useEffect(() {
      return tabController.dispose;
    }, const []);

    return Column(
      children: [
        Container(
          height: 45,
          decoration: BoxDecoration(
            color: Colors.grey.shade800,
            boxShadow: [
              BoxShadow(
                color: Colors.black.withOpacity(0.5),
                spreadRadius: 5,
                blurRadius: 7,
                offset: Offset(0, 3),
              )
            ],
          ),
          child: TabBar(
            tabs: tabs,
            indicator: BoxDecoration(
              color: Colors.green,
            ),
            controller: tabController,
          ),
        ),
        Expanded(
          child: Container(
            child: TabBarView(
              children: tabWidgets,
              controller: tabController,
            ),
            decoration: BoxDecoration(
              color: scaffoldBackgroundColorCodes[900],
              border: Border(
                top: BorderSide(
                  color: Colors.black,
                  width: 2.4,
                ),
              ),
              boxShadow: [
                BoxShadow(
                  color: Colors.black.withOpacity(0.5),
                  spreadRadius: 5,
                  blurRadius: 7,
                  offset: Offset(0, 3),
                )
              ],
            ),
          ),
        ),
      ],
    );
  }
}
