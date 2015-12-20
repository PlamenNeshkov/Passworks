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
  });

  $scope.receivedData = [];
  $scope.$watch('serialService.getData()', function(newVal) {
    $scope.receivedData = newVal;
  });

  SerialService.onReceive();

  $scope.accounts = [
    {
      type: "Steam",
      username: "pesho1",
      password: "123"
    },
    {
      type: "Google",
      username: "gosho2",
      password: "456"
    },
    {
      type: "Facebook",
      username: "ivancho3",
      password: "789"
    }
  ];

  $scope.createAccount = function(account) {
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
