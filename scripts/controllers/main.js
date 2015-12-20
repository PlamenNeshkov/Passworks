'use strict';

app.controller('mainCtrl', function($scope, SerialService) {
  $scope.serialService = SerialService;

  $scope.connected = false;
  $scope.$watch('serialService.isConnected()', function(newVal) {
    $scope.connected = newVal;
    console.log("Connected: " + newVal);
  });

  $scope.authenticated = false;
  $scope.$watch('serialService.isAuthenticated()', function(newVal) {
    $scope.authenticated = newVal;
    console.log("Authenticated: " + newVal);
    if (newVal) {
      SerialService.populate();
    }
  });

  $scope.receivedData = [];
  $scope.$watch('serialService.getData()', function(newVal) {
    $scope.receivedData = newVal;
  });

  SerialService.onReceive();

  $scope.accounts = [];
  $scope.$watch('serialService.getAccounts()', function(newVal) {
    $scope.accounts = newVal;
  });

  $scope.createAccount = function(account) {
    var pad = "                ";

    var id = $scope.accounts.length;
    var type = (pad + account.type).slice(-16);
    var username = (pad + account.username).slice(-16);
    var password = (pad + account.password).slice(-16);
    console.log("Saving new account...");
    SerialService.writeSerial("SAVE" + "&" + id + "&" + type
                              + "&" + username + "&" + password);
    $scope.accounts.push(account);
    $scope.creating = false;
  }

  $scope.updateAccount = function(account, $index) {
    $scope.accounts[$index] = account;
    account.updating = false;
  }

  $scope.deleteAccount = function($index) {
    $scope.accounts.splice($index, 1);
  }
});
