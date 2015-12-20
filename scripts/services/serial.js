app.service('SerialService', function($rootScope) {
  var self = this;

  var connected = false;
  var authenticated = false;

  this.isConnected = function() {
    return connected;
  };
  this.isAuthenticated = function() {
    return authenticated;
  }

  // Connecting
  var connectionId = -1;

  this.onConnect = function(connectionInfo) {
    connectionId = connectionInfo.connectionId;
    self.writeSerial("NG-INIT-HANDSHAKE");
  };

  this.connect = function(path) {
    chrome.serial.connect(path, {}, this.onConnect);
  };

  // Disconnecting
  this.onDisconnect = function(result) {
    if (result) {
      connectionId = -1;
      console.log("Successfully disconnected, id: " + connectionId);
    } else {
      console.log("Disconnect failed");
    }
  };

  this.disconnect = function() {
    chrome.serial.disconnect(connectionId, this.onDisconnect);
  };

  // Reading
  var currentData = "";
  var receivedData = [];

  this.getData = function() {
    return receivedData;
  }

  this.buf2str = function(buf) {
    var bufView = new Uint8Array(buf);
    var encodedString = String.fromCharCode.apply(null, bufView);
    return decodeURIComponent(escape(encodedString));
  };

  this.onReceiveCallback = function(info) {
    if (info.connectionId == connectionId && info.data) {
      var str = self.buf2str(info.data);
      if (str.charAt(str.length-1) === '\n') {
        currentData += str.substring(0, str.length - 1);
        receivedData.push(currentData);
        $rootScope.$apply();
        currentData = "";
      } else {
        currentData += str;
      }
    }
  };

  this.onReceive = function() {
    chrome.serial.onReceive.addListener(this.onReceiveCallback);
  }

  //Writing

  this.str2buf = function(str) {
    var buf=new ArrayBuffer(str.length);
    var bufView=new Uint8Array(buf);
    for (var i=0; i<str.length; i++) {
      bufView[i]=str.charCodeAt(i);
    }
    return buf;
  };

  this.onSend = function() {
    $rootScope.$watch('self.getData()', function handleChange(newValue, oldValue) {
      var reply = receivedData[receivedData.length - 1].replace(/[\r\n]/g, "");
      if (reply == "PASSWORKS") {
        connected = true;
      } else if (reply == "AUTH-SUCCESS") {
        authenticated = true;
      }
    });
  };

  this.writeSerial = function(str) {
    chrome.serial.send(connectionId, self.str2buf(str + '\n'), self.onSend);
  };

  // Devices
  var devices = [];

  this.onGetDevices = function(ports) {
    console.log("Attempting to connect to device...");
    angular.forEach(ports, function(value, index) {
      devices.push(value.path);
      if (connected == false) {
        self.connect(value.path);
      }
    });
  };



  this.getDevices = function() {
    chrome.serial.getDevices(this.onGetDevices);
  };

  // Auth
  this.authenticate = function(user, pass) {
    self.writeSerial("AUTH" + "&" + user + "&" + pass);
  };


  this.getAcc = function(name)
  {
    self.writeSerial("GET");
  };
});
