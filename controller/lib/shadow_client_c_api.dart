library c_api;

import 'dart:isolate';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';

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
    ffi.Int64 port9,
    ffi.Int64 port10,
    ffi.Int64 port11);

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
    int port9,
    int port10,
    int port11);

//FFI signature for void function
typedef Void_Function_FFI = ffi.Void Function();
//Dart type definition for calling the C foreign function
typedef Void_Function_C = void Function();

typedef StartSimFFI = ffi.Void Function(ffi.Uint32 configuration);
typedef StartSim_C = void Function(int configuration);

typedef GetVMSnapsFFI = ffi.Void Function(ffi.Pointer<Utf8> vm);
typedef hello_world = ffi.Pointer<Utf8> Function();

typedef GetVMSnapsC = ffi.Void Function(
    ffi.Pointer<Utf8> str, ffi.Int32 length);
typedef GetVMSnapsDart = void Function(ffi.Pointer<Utf8> str, int length);

typedef RestoreSnapC = ffi.Void Function(ffi.Pointer<Utf8> vm,
    ffi.Int32 vm_length, ffi.Pointer<Utf8> snap, ffi.Int32 snapLength);

typedef RestoreSnapDart = void Function(
    ffi.Pointer<Utf8> vm, int vmLength, ffi.Pointer<Utf8> snap, int snapLength);

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
  late GetVMSnapsDart getVMsnaps;
  late RestoreSnapDart restoreSnap;

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

    ReceivePort snapNamesPort = ReceivePort()
      ..listen((data) {
        print("snaps port: ${data.runtimeType}");
        if (data.runtimeType == int) {
          tempSnapNamesInnerList.clear();
          tempSnapNameSize = data;
          print("got size of: $data, $tempSnapNameSize");
        } else {
          tempSnapNamesInnerList.add(data);
        }

        if (tempSnapNamesInnerList.length == tempSnapNameSize) {
          print("got completed list of size: $tempSnapNameSize");
          tempSnapNamesList.add(List<String>.from(tempSnapNamesInnerList));
          tempSnapNamesInnerList.clear();
        }

        print(tempSnapNamesList.length);

        if (tempSnapNamesList.length == vmList.length) {
          print("tsnl size: ${tempSnapNamesList.length}");
          snapNamesList = List<List<String>>.from(tempSnapNamesList);
          tempSnapNamesList.clear();
        }
        notifyListeners();
      });

    int snapNamesNativePort = snapNamesPort.sendPort.nativePort;

    ReceivePort snapDescPort = ReceivePort()
      ..listen((data) {
        print("snap desc port: ${data.runtimeType}");
        if (data.runtimeType == int) {
          tempSnapDescInnerList.clear();
          tempSnapDescSize = data;
          print("got size of: $data, $tempSnapDescSize");
        } else {
          tempSnapDescInnerList.add(data);
        }

        if (tempSnapDescInnerList.length == tempSnapDescSize) {
          print("got completed list of size: $tempSnapDescSize");
          tempSnapDescList.add(List<String>.from(tempSnapDescInnerList));
          tempSnapDescInnerList.clear();
        }

        print(tempSnapDescList.length);

        if (tempSnapDescList.length == vmList.length) {
          print("tsnl size: ${tempSnapNamesList.length}");
          snapDescList = List<List<String>>.from(tempSnapDescList);
          tempSnapDescList.clear();
        }
        notifyListeners();
      });

    int snapDescNativePort = snapDescPort.sendPort.nativePort;

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

    getVMsnaps =
        lib.lookupFunction<GetVMSnapsC, GetVMSnapsDart>('get_vm_snaps');

    restoreSnap = lib.lookupFunction<RestoreSnapC, RestoreSnapDart>("restore");

    final backwards = 'backwards';
    getVMsnaps(backwards.toNativeUtf8(), backwards.length);

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
        vmRunningNativePort,
        snapNamesNativePort,
        snapDescNativePort);
    runIOService();
  }

  static const String _LIBRARY_NAME =
      '/home/efsi/projects/dev/save_and_restore/controller/shadow_client_backend/build/libshadow_client_backend.so';

  List<int>? tempVMrunningList;
  List<String> tempVMList = List.empty(growable: true);

  List<int>? vmRunningList;
  List<String> vmList = List.empty(growable: true);

  List<String> tempSnapNamesInnerList = List.empty(growable: true);
  List<List<String>> tempSnapNamesList = List.empty(growable: true);
  int tempSnapNameSize = 0;

  List<String> tempSnapDescInnerList = List.empty(growable: true);
  List<List<String>> tempSnapDescList = List.empty(growable: true);
  int tempSnapDescSize = 0;

  List<List<String>> snapNamesList = List.empty(growable: true);
  List<List<String>> snapDescList = List.empty(growable: true);
}

ShadowClientCAPI? shadowClientCAPI;
