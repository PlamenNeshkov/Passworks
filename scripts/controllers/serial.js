'use strict';

app.controller('serialCtrl', function($scope) {
  // // Reading
  // $scope.currentData = "";
  // $scope.receivedData = [];
  //
  // $scope.buf2str = function(buf) {
  //   var bufView = new Uint8Array(buf);
  //   var encodedString = String.fromCharCode.apply(null, bufView);
  //   return decodeURIComponent(escape(encodedString));
  // };
  //
  // $scope.onReceiveCallback = function(info) {
  //   $scope.$apply(function() {
  //     if (info.connectionId == $scope.connectionId && info.data) {
  //       var str = $scope.buf2str(info.data);
  //       if (str.charAt(str.length-1) === '\n') {
  //         $scope.currentData += str.substring(0, str.length - 2);
  //         $scope.receivedData.push($scope.currentData);
  //         $scope.currentData = "";
  //       } else {
  //         $scope.currentData += str;
  //       }
  //     }
  //   });
  // };
  //
  // $scope.onReceive = function() {
  //   chrome.serial.onReceive.addListener($scope.onReceiveCallback);
  // }
  // $scope.onReceive();

  // $scope.connected = false;
  // // $scope.authenticated = false;
  //
  // $scope.getInfo = function() {
  //   chrome.serial.getInfo($scope.connectionId, function(connectionInfo) {
  //     console.log(connectionInfo);
  //   });
  // }

  // Device listing
  // $scope.devices = [];
  //
  // $scope.onGetDevices = function(ports) {
  //   var devices = [];
  //
  //   //angular.forEach(ports, function(value, index) {
  //    devices.push(ports[0].path);
  //    $scope.connect(ports[0].path);
  //    $scope.checkConnection();
  //   //});
  //
  //   $scope.devices = devices;
  // };
  //
  // $scope.getDevices = function() {
  //   chrome.serial.getDevices($scope.onGetDevices);
  // };

  // // Connecting
  // $scope.connectionId = -1;
  //
  // $scope.onConnect = function(connectionInfo) {
  //   $scope.connectionId = connectionInfo.connectionId;
  //   console.log("Successful connection; id: " + $scope.connectionId);
  // };
  //
  // $scope.connect = function(path) {
  //   chrome.serial.connect(path, {}, $scope.onConnect);
  // };

  $scope.checkConnection = function() {
    $scope.writeSerial("NG-INIT-HANDSHAKE&");
    console.log("test1");
    console.log($scope.receivedData);
    $scope.$watch("receivedData", function handleChange(newValue, oldValue) {
      console.log("test2");
      console.log($scope.receivedData);
      if ($scope.receivedData[$scope.receivedData.length - 1] == "PASSWORKS") {
        $scope.connected = true;
      }
    });
  //};

  // // Disconnecting
  // $scope.onDisconnect = function(result) {
  //   if (result) {
  //     console.log("Successfully disconnected from serial port.");
  //     $scope.$apply(function() {
  //       $scope.connectionId = -1;
  //     });
  //   } else {
  //     console.log("Disconnect failed.");
  //   }
  // };
  //
  // $scope.disconnect = function() {
  //   chrome.serial.disconnect($scope.connectionId, $scope.onDisconnect);
  // };

  // //Writing
  //
  // $scope.str2buf = function(str) {
  //   var buf=new ArrayBuffer(str.length);
  //   var bufView=new Uint8Array(buf);
  //   for (var i=0; i<str.length; i++) {
  //     bufView[i]=str.charCodeAt(i);
  //   }
  //   return buf;
  // };
  //
  // $scope.onSend = function() {
  //   $scope.$watch('receivedData', function handleChange(newValue, oldValue) {
  //     console.log($scope.receivedData);
  //     if ($scope.receivedData[$scope.receivedData.length - 1] == "AUTH-SUCCESS") {
  //       $scope.authenticated = true;
  //     }
  //   });
  // };
  //
  // $scope.writeSerial = function(str) {
  //   chrome.serial.send($scope.connectionId, $scope.str2buf(str + '\n'), $scope.onSend);
  // };

  //Flush
  $scope.flushSerial = function() {
    chrome.serial.flush($scope.connectionId, function() {});
  };

  $scope.authenticate = function() {
    var user = $scope.credentials.username;
    var pass = $scope.credentials.password;
    $scope.writeSerial("AUTH" + "&" + user + "&" + pass);
  };
});
