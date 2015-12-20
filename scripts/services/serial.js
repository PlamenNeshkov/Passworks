app.service('SerialService', function($rootScope) {
  var self = this;

  var connected = false;
  var authenticated = false;

  var accounts = [];
  this.accountNum = 0;

  this.getAccounts = function() {
    return accounts;
  };

  this.isConnected = function() {
    return connected;
  };
  this.isAuthenticated = function() {
    return authenticated;
  };

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
  $rootScope.receivedData = [];

  this.getData = function() {
    return self.receivedData;
  };

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
        console.log(currentData);

        var reply = currentData.replace(/[\r\n]/g, "");

        if (reply == "PASSWORKS") {
          console.log("connected");
          connected = true;
        } else if (reply == "AUTH-SUCCESS") {
          authenticated = true;
        } else if (reply.split("¦")[0] == "ALL") {
          console.log(reply);
        } else if (reply.split(" ")[0] == "NUM") {
          self.accountNum = reply.split(" ")[1];
          console.log("New acc num");
        }
        $rootScope.$apply();
        currentData = "";
      } else {
        currentData += str;
      }
    }
  };

  this.onReceive = function() {
    chrome.serial.onReceive.addListener(this.onReceiveCallback);
  };
  this.onReceive();

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
  };

  this.writeSerial = function(str) {
    chrome.serial.send(connectionId, self.str2buf(str + '\n'), self.onSend);
  };

  // Devices
  var devices = [];

  this.onGetDevices = function(ports) {
    console.log("Getting devices...");
    angular.forEach(ports, function(value, index) {
      devices.push(value.path);
      if (connected == false) {
        self.connect(value.path);
      }
    });
    //self.connect(ports[0].path);
  };

  this.getDevices = function() {
    chrome.serial.getDevices(this.onGetDevices);
  };
  this.getDevices();

  // Auth
  this.authenticate = function(user, pass) {
    self.writeSerial("AUTH" + "&" + user + "&" + pass);
  };

  this.getAcc = function(id) {
   self.writeSerial("GET" + "&" + id);
  };

  this.populate = function() {
    console.log("Retrieving all accounts");
    console.log(self.accountNum);
    self.getAcc(0);
  }
});
