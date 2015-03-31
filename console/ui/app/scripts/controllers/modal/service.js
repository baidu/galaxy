angular.module('galaxy.ui.modal.service',[]).controller('ShowCreateServiceModalCtrl', function ($scope, $modal, $log) {
  $scope.open = function (size) {
    var modalInstance = $modal.open({
      templateUrl: 'views/createService.html',
      controller: 'CreateServiceModalInstanceCtrl',
      keyboard:false,
      size: size
    });
    modalInstance.result.then(function (selectedItem) {
      $scope.selected = selectedItem;
    }, function () {
    });
  };
});

// Please note that $modalInstance represents a modal window (instance) dependency.
// It is not the same as the $modal service used above.

angular.module('galaxy.ui.modal.service').controller('CreateServiceModalInstanceCtrl', function ($scope, $modalInstance) {
  $scope.ok = function () {
    $modalInstance.close($scope.selected.item);
  };
  $scope.service = {}
  $scope.service.name = "hello";

  $scope.service.data = {
    "packageSource": "ftp://yf-matrix-op.yf01.baidu.com:/home/work/matrix/public/matrix_example/hello-baidu.tar.gz",
    "packageVersion": "1.0",
    "packageType": 0,
    "user": "work",
    "deployDir": "/home/work/hello/",
    "process": {},
    "port":{"staticPort": {"main": 8547}, "dynamicPortName": []},
    "tag": {"tag1": "value12"},
    "deployTimeoutSec": 30,
    "resource":{},
    "healthCheckTimeoutSec": 30,
    "hostConstrain": [],
    "group":"test",
    "resourceRequirement": {
        "cpu": {"normalizedCore": {"quota": 5, "limit": -1}},
        "memory": {"sizeMB": {"quota": 200, "limit": -1}},
        "network": {"inBandwidthMBPS": {"quota": 1, "limit": -1}, "outBandwidthMBPS": {"quota": 1, "limit": -1}},
        "port": {"staticPort": {"main": 8547}, "dynamicPortName": []},
        "process": {"thread": {"quota": 100, "limit": -1}},
        "workspace": {"sizeMB": {"quota": 100, "limit": -1}, "inode": {"quota": 100, "limit": -1}, "type": "home", "bindPoint": "", "exclusive": false, "useSoftLinkDir": false},
        "requiredDisk": [],
        "optionalDisk": []
    },
    "baseEnv": "system",
    "priority": 0
};
  $scope.service.dataStr = JSON.stringify($scope.service.data,null, 4);
  $scope.$watch('service.dataStr', function(json) {
        try {
           $scope.service.data = JSON.parse(json);
        } catch(e) {
        }
  });

  $scope.cancel = function () {
    $modalInstance.dismiss('cancel');
  };
  $scope.step1style = 'glyphicon glyphicon-ok stepdone';
});