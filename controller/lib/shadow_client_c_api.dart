library c_api;

import 'dart:isolate';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'dart:ffi' as ffi;

typedef StartWorkType = ffi.Void Function(
    ffi.Uint8 using_dart,
    ffi.Int64 port1,
    ffi.Int64 port2,
    ffi.Int64 port3,
    ffi.Int64 port4,
    ffi.Int64 port5,
    ffi.Int64 port6,
    ffi.Int64 port7,
    ffi.Int64 port8,
    ffi.Int64 port9);

typedef StartWorkFunc = void Function(
    int using_dart,
    int port1,
    int port2,
    int port3,
    int port4,
    int port5,
    int port6,
    int port7,
    int port8,
    int port9);

//FFI signature for void function
typedef Void_Function_FFI = ffi.Void Function();
//Dart type definition for calling the C foreign function
typedef Void_Function_C = void Function();

typedef StartSimFFI = ffi.Void Function(ffi.Uint32 configuration);
typedef StartSim_C = void Function(int configuration);

class ShadowClientCAPI extends ChangeNotifier {
  bool serverConnected = false;
  bool controlOfSatellite = false;
  bool simulationStarted = false;
  bool simicsPlaying = false;
  bool vmRunning = false;

  late Void_Function_C takeControlOfSatellite;
  late StartSim_C startSimulation;
  late Void_Function_C stopSimulation;
  late Void_Function_C startSimics;
  late Void_Function_C pauseSimics;

  void resetAll() {
    serverConnected = false;
    controlOfSatellite = false;
    simulationStarted = false;
    simicsPlaying = false;
    vmRunning = false;
    vmRunningList = null;
    vmList.clear();
  }

  ShadowClientCAPI() {
    print("created shadow client");
    shadowClientCAPI = this;

    final lib = ffi.DynamicLibrary.open(_LIBRARY_NAME);

    final initializeApi = lib.lookupFunction<
        ffi.IntPtr Function(ffi.Pointer<ffi.Void>),
        int Function(ffi.Pointer<ffi.Void>)>("InitializeDartApi");

    if (initializeApi(ffi.NativeApi.initializeApiDLData) != 0) {
      throw "Failed to initialize Dart API";
    }

    ReceivePort applicationInitialStatusPort = ReceivePort()
      ..listen((data) {
        print('received application initial status ${data.length}');
      });
    int applicationInitialStatusNativePort =
        applicationInitialStatusPort.sendPort.nativePort;

    ReceivePort applicationInitialNamesPort = ReceivePort()
      ..listen((name) {
        print('received name: $name');
      });
    int applicationInitialNamesNativePort =
        applicationInitialNamesPort.sendPort.nativePort;

    ReceivePort connectionPort = ReceivePort()
      ..listen((status) {
        print('connection port status change: $status');
        if (status != serverConnected) {
          serverConnected = status;
          if (serverConnected == false) resetAll();
          notifyListeners();
        }
      });
    int connectionNativePort = connectionPort.sendPort.nativePort;

    ReceivePort controlStatusPort = ReceivePort()
      ..listen((status) {
        print('control status received name: $status');
        if (status != controlOfSatellite) {
          controlOfSatellite = status;
          notifyListeners();
        }
      });
    int controlStatusNativePort = controlStatusPort.sendPort.nativePort;

    ReceivePort simStatusPort = ReceivePort()
      ..listen((status) {
        print('sim status received name: $status');
        if (status != simulationStarted) {
          simulationStarted = status;
          notifyListeners();
        }
      });
    int simStatusNativePort = simStatusPort.sendPort.nativePort;

    ReceivePort simicsStatusPort = ReceivePort()
      ..listen((status) {
        print('simics received name: $status');
        if (status != simicsPlaying) {
          simicsPlaying = status;
          notifyListeners();
        }
      });
    int simicsStatusNativePort = simicsStatusPort.sendPort.nativePort;

    ReceivePort vmRunningListPort = ReceivePort()
      ..listen((data) {
        //print('received vm running list status ${data.length}, type: ${data.runtimeType}');
        tempVMrunningList = data.toList();
      });
    int vmRunningListNativePort = vmRunningListPort.sendPort.nativePort;

    ReceivePort vmListPort = ReceivePort()
      ..listen((name) {
        //print('received name: $name');
        tempVMList.add(name);
        //print("tempVMList length: ${tempVMList.length}");
        if (tempVMrunningList != null) {
          //print("tempVMRunninList length: ${tempVMrunningList?.length}");
        }

        if (tempVMList.length == tempVMrunningList?.length) {
          //print("they are both the same size!");
          vmList = List<String>.from(tempVMList);
          vmRunningList =
              List<int>.from(tempVMrunningList ?? List<int>.empty());

          tempVMList.clear();
          tempVMrunningList?.clear();

          //print("vmList length: ${vmList.length}");
          //print("vmRunningList length: ${vmRunningList?.length}");
          notifyListeners();
        }
      });

    int vmListNativePort = vmListPort.sendPort.nativePort;

    ReceivePort vmRunningPort = ReceivePort()
      ..listen((status) {
        print("vm running: $status");
      });

    int vmRunningNativePort = vmRunningPort.sendPort.nativePort;

    StartWorkFunc client = lib
        .lookup<ffi.NativeFunction<StartWorkType>>("create_client")
        .asFunction();

    Void_Function_C runIOService = lib
        .lookup<ffi.NativeFunction<Void_Function_FFI>>("run_service")
        .asFunction();

    takeControlOfSatellite = lib
        .lookup<ffi.NativeFunction<Void_Function_FFI>>("take_control")
        .asFunction();

    startSimulation = lib
        .lookup<ffi.NativeFunction<StartSimFFI>>("start_simulation")
        .asFunction();

    stopSimulation = lib
        .lookup<ffi.NativeFunction<Void_Function_FFI>>("stop_simulation")
        .asFunction();

    startSimics = lib
        .lookup<ffi.NativeFunction<Void_Function_FFI>>("start_simics")
        .asFunction();

    pauseSimics = lib
        .lookup<ffi.NativeFunction<Void_Function_FFI>>("pause_simics")
        .asFunction();

    client(
        1,
        applicationInitialStatusNativePort,
        applicationInitialNamesNativePort,
        connectionNativePort,
        controlStatusNativePort,
        simStatusNativePort,
        simicsStatusNativePort,
        vmRunningListNativePort,
        vmListNativePort,
        vmRunningNativePort);
    runIOService();
  }

  static const String _LIBRARY_NAME =
      '/home/efsi/projects/dev/save_and_restore/controller/shadow_client_backend/build/libshadow_client_backend.so';

  List<int>? tempVMrunningList;
  List<String> tempVMList = List.empty(growable: true);

  List<int>? vmRunningList;
  List<String> vmList = List.empty(growable: true);
}

ShadowClientCAPI? shadowClientCAPI;
