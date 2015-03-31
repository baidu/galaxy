(function(angular){


angular.module('galaxy.ui.ctrl')
       .controller('ServiceCtrl',['$scope','$routeParams','$modal','$http','$log','$interval','notify','$route',function ($scope,$routeParams,$modal,$http,$log,$interval,notify,$route) {
      var instanceMetaTemplate = {
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
     $scope.hideInitServForm= true;
     $scope.initServFormData = {servName:$routeParams.serviceName,instanceCount:0,meta:JSON.stringify(instanceMetaTemplate,null, 4)};
     $scope.serviceName = $routeParams.serviceName;
     $scope.openModal = function (size) {
      var modalInstance = $modal.open({
        templateUrl: 'views/instanceStatus.html',
        controller: 'InstanceStatusCtrl',
        keyboard:false,
        backdrop:'static',
        size: size,
        scope:$scope
      });
     
    };

    $scope.showInstance = function(instance){
        var modalInstance = $modal.open({
        templateUrl: 'views/showInstance.html',
        controller: 'ShowInstanceCtrl',
        keyboard:false,
        backdrop:true,
        size: 'sm',
        scope:$scope,
        resolve:{
            instance:function(){
                return instance;
            }
        }
      });
    }

    $scope.addInstance = function(){
        var modalInstance = $modal.open({
        templateUrl: 'views/addInstance.html',
        controller: 'AddInstanceCtrl',
        keyboard:false,
        backdrop:'static',
        scope:$scope,
        resolve:{
            instanceCount:function(){
                 return $scope.allInstanceCount ;
            },
            metaTpl:function(){
                if($scope.config != null){
                    return JSON.stringify(JSON.parse($scope.config.meta),null, 4);    
                }else{
                    return JSON.stringify(instanceMetaTemplate,null, 4);    
                }
                
            }
        }
      });
    }
    $scope.stop = undefined;
    $scope.hideSpinner = false;
    $scope.config = null;
   
    $scope.getStatus = function(){
        $http.get("/ajax/instance/getAllInstance?name="+$scope.serviceName).success(function(data, status, headers, config){
                if(data.status != 0 ){
                    $log.error("fail to instance list for "+ data.msg);
                    return ;
                }
                var noNeedRefreshCount = 0;
                var instanceList = [];
                var allInstanceCount = 0;
                if(data.data==null || data.data.length == 0){
                    $scope.hideInitServForm = false;
                }else{
                    $scope.hideInitServForm = true;
                }
                for(var index in  data.data){
                    var groupInstance = data.data[index].instanceGroup;
                    $scope.config = data.data[index].configHistory;
                    var bInstanceList = data.data[index].instanceList;
                    if (bInstanceList==null){
                        continue;
                    }
                    allInstanceCount += bInstanceList.length;
                    for(var iindex in bInstanceList){
                         var instanceDetail = bInstanceList[iindex];
                         switch(instanceDetail.state){
                         case 0:  instanceList.push({tip:"deploying",group:"G"+groupInstance.offset,label:'D',offset:instanceDetail.offset,state:"deploy-instance",data:instanceDetail});break;
                         case 10: instanceList.push({tip:"pending",group:"G"+groupInstance.offset,label:'P',offset:instanceDetail.offset,state:"pending-instance",data:instanceDetail});break;
                         case 20: instanceList.push({tip:"starting",group:"G"+groupInstance.offset,label:'S',offset:instanceDetail.offset,state:"starting-instance",data:instanceDetail});break;
                         case 30: instanceList.push({tip:"running",group:"G"+groupInstance.offset,label:'R',offset:instanceDetail.offset,state:"running-instance",data:instanceDetail});noNeedRefreshCount+=1;break;
                         case 40: instanceList.push({tip:"error",group:"G"+groupInstance.offset,label:'E',offset:instanceDetail.offset,state:"error-instance",data:instanceDetail});noNeedRefreshCount+=1;break;
                         case 50: instanceList.push({tip:"removing",group:"G"+groupInstance.offset,label:'RM',offset:instanceDetail.offset,state:"removing-instance",data:instanceDetail});$scope.canRefresh=true;break;
                        }
                    }
                }
                $scope.allInstanceCount = allInstanceCount;
                $scope.instanceList = instanceList;
                if(noNeedRefreshCount == allInstanceCount){
                    if (angular.isDefined($scope.stop)) {
                        $interval.cancel($scope.stop);
                        $scope.stop = undefined;
                    }
                     $scope.hideSpinner = true;
                }
    });
    }
    $scope.hideSpinner = false;
    $scope.stop = $interval($scope.getStatus,500);
    $scope.initServ = function(){
        $http({method:"POST",
                url:"/ajax/instance/init",
                data: $scope.initServFormData ,
                headers:{'Content-Type': 'application/x-www-form-urlencoded'},
                transformRequest: function(obj) {
                          var str = [];
                          for(var p in obj){
                             str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
                          }
                          return str.join("&");
                }
            }).success(function(data, status, headers, config) {
                if(data.status == 0 ){
                     notify({ message:'初始化服务成功'} );
                     $scope.hideSpinner = false;
                     $scope.stop = $interval($scope.getStatus,500);
                }else{
                    notify({ message:'初始化服务失败：'+data.msg,classes:"alert-danger"} );
                }
                $scope.disableBtn = false;
            });
    }

  }]);



angular.module('galaxy.ui.ctrl').controller('ShowInstanceCtrl',function($scope,$modalInstance,instance,config){
    $scope.instance = instance;
    $scope.config = config.config;
    $scope.cancel = function(){
        $modalInstance.dismiss('cancel');
    }

});
angular.module('galaxy.ui.ctrl').controller('AddInstanceCtrl',function($scope,$modalInstance,$interval,$log,$http,notify,instanceCount,metaTpl){
    $scope.formdata = {name:$scope.$parent.serviceName,updateMeta:false,count:instanceCount,meta:metaTpl}
    $scope.cancel = function(){
        $modalInstance.dismiss('cancel');
    }
    $scope.ok = function(){
        $scope.disableBtn = true;
         $http({method:"POST",
                url:"/ajax/instance/update",
                data:$scope.formdata  ,
                headers:{'Content-Type': 'application/x-www-form-urlencoded'},
                transformRequest: function(obj) {
                          var str = [];
                          for(var p in obj){
                             str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
                          }
                          return str.join("&");
                }
            }).success(function(data, status, headers, config) {
                if(data.status == 0 ){
                    notify({ message:'更新实例数成功'} );
                    $scope.$parent.hideSpinner = false;
                    $scope.$parent.stop = $interval($scope.getStatus,500);
                    $scope.cancel();

                }else{
                    notify({ message:'添加失败实例数',classes:"alert-danger"} );
                }
                $scope.disableBtn = false;
            });
    }



});
angular.module('galaxy.ui.ctrl').filter('propsFilter', function() {
  return function(items, props) {
    var out = [];

    if (angular.isArray(items)) {
      items.forEach(function(item) {
        var itemMatches = false;

        var keys = Object.keys(props);
        for (var i = 0; i < keys.length; i++) {
          var prop = keys[i];
          var text = props[prop].toLowerCase();
          if (item[prop].toString().toLowerCase().indexOf(text) !== -1) {
            itemMatches = true;
            break;
          }
        }

        if (itemMatches) {
          out.push(item);
        }
      });
    } else {
      // Let the output be the input untouched
      out = items;
    }

    return out;
  }
});


}(angular))
