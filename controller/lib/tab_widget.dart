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

    useEffect(() {
      return tabController.dispose;
    }, const []);

    return Container(
      child: Container(
        child: Column(
          children: [
            Container(
              height: _tabHeight,
              color: _tabUnselectedColor,
              child: TabBar(
                tabs: tabs,
                controller: tabController,
                indicator: _tabIndicator,
              ),
            ),
            Expanded(
              child: Container(
                decoration: BoxDecoration(
                  color: scaffoldBackgroundColor,
                  boxShadow: [
                    BoxShadow(
                      blurRadius: _BLUR_RADIUS,
                      spreadRadius: _SPREAD_RADIUS,
                      color: _boxShadowColor,
                      offset: _boxShadowOffset,
                    )
                  ],
                ),
                child: TabBarView(
                  children: tabWidgets,
                  controller: tabController,
                ),
              ),
            )
          ],
        ),
      ),
    );
  }

  static const double _tabHeight = 45;
  static Color _tabUnselectedColor = Colors.grey.shade800;
  static Decoration _tabIndicator = BoxDecoration(color: Colors.blue.shade900);

  //box shadow settings
  static const double _BLUR_RADIUS = 5;
  static const double _SPREAD_RADIUS = 4;
  static Color _boxShadowColor = Colors.black.withOpacity(0.8);
  static Offset _boxShadowOffset = Offset(0, 10);
}
