'use strict';

/**
 * @ngdoc function
 * @name uidemoApp.controller:MainCtrl
 * @description
 * # MainCtrl
 * Controller of the uidemoApp
 */
angular.module('galaxy.ui.ctrl',[])
  .controller('HomeCtrl', function ($scope,$modal,$http,$route,notify,$log) {
     $scope.open = function (size) {
      var modalInstance = $modal.open({
        templateUrl: 'views/createService.html',
        controller: 'CreateServiceModalInstanceCtrl',
        keyboard:false,
        backdrop:'static',
        size: size
      });
    };
    $http.get("/service/list?user=9527")
         .success(function(data){
          if(data.status == 0 ){
               $scope.serviceList = data.data;  
          }else{
            notify({ message:'获取服务列表失败',classes:"alert-danger"} );
            $log.error(data.msg);
          }
         }).error(function(data){
            notify({ message:'获取服务列表失败',classes:"alert-danger"} );
            $log.error(data);
     });
    $scope.deleteService = function(id){
      var promot = $modal.open({
        templateUrl: 'views/promot.html',
        controller: 'PromotCtrl',
        keyboard:false,
        backdrop:'static',
        size: 'sm',
        resolve:{
          message:function(){
            return "确定删除服务？";
          }
        }
      });
      promot.result.then(function(result){
        if(result){
          $http.get("/service/delete?service="+id)
               .success(function(data){
                  if(data.status == 0){
                    notify({ message:'删除服务成功'} );
                    $route.reload();  
                  }else{
                    notify({ message:'删除服务失败',classes:"alert-danger"} );
                    $log.error(data.msg);
                  }
          
                })
               .error(function(data){
                     notify({ message:'删除服务失败',classes:"alert-danger"} );
                     $log.error(data);
               });
        }
      },function(){});
    }
      
  });

angular.module('galaxy.ui.ctrl').controller('CreateServiceModalInstanceCtrl', 
                                            function ($scope, $modalInstance,$http,$route,notify) {

  $scope.disableBtn=false;
  $scope.alerts = [];
  $scope.service = {user:"9527",delayMigrateTimeSec:1000,name:"",enableSchedule:false,migrateRetryCount:3,migrateStopThreshold:10,enableNaming:false};
  $scope.ok = function () {
    $scope.alerts = [];
    $scope.disableBtn=true;
    $http(
      {
        method:"POST",
        url:'/service/create', 
        data:$scope.service,
        headers:{'Content-Type': 'application/x-www-form-urlencoded'},
        transformRequest: function(obj) {
          var str = [];
          for(var p in obj){
             str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
          }
          return str.join("&");
        }
      }).success(function(data, status, headers, config) {
        $scope.disableBtn=false;
       if(data.status==0){
         notify({ message:'创建服务成功'} );
         $route.reload();
         $modalInstance.close();
       }else{
           $scope.alerts.push({ type: 'danger', msg: '错误：'+data.msg });
       }
     }).error(function(data, status, headers, config) {
        $scope.disableBtn=false;

     });
    //$modalInstance.close($scope.selected.item);
  };
  $scope.cancel = function () {
    $modalInstance.dismiss('cancel');
  };
  $scope.closeAlert = function(index) {
    $scope.alerts.splice(index, 1);
  };
  
  
});

