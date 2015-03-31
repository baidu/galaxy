angular.module('galaxy.ui.ctrl')
       .controller('ResourceCtrl',function($scope,$http,notify,$log,$modal,$route){
           $scope.resourceList = [];
           $http.get("/ajax/resource/list")
                .success(function(data){
                    if(data.status == 0 ){
                        $scope.resourceList = data.data;
                    }else{
                        notify({ message:'获取服务列表失败',classes:"alert-danger"} );
                        $log.error(data);
                    }
                })
                .error(function(data){
                    $log.error(data);
                });
            $scope.addResource = function () {
                  var modalInstance = $modal.open({
                        templateUrl: 'views/addResource.html',
                        controller: 'AddResourceModalCtrl',
                        keyboard:false,
                        backdrop:'static'
                  });
             };
             $scope.deleteService = function(id){
              var promot = $modal.open({
                templateUrl: 'views/promot.html',
                controller: 'PromotCtrl',
                keyboard:false,
                backdrop:'static',
                size: 'sm',
                resolve:{
                  message:function(){
                    return "确定删除资源？";
                  }
                }
              });
              promot.result.then(function(result){
                if(result){
                  $http.get("/ajax/resource/delete?id="+id)
                       .success(function(data){
                          if(data.status == 0){
                            notify({ message:'删除资源成功'} );
                            $route.reload();  
                          }else{
                            notify({ message:'删除资源失败',classes:"alert-danger"} );
                            $log.error(data.msg);
                          }
                  
                        })
                       .error(function(data){
                             notify({ message:'删除资源失败',classes:"alert-danger"} );
                             $log.error(data);
                       });
                }
              },function(){});
              }
      });

angular.module('galaxy.ui.ctrl').controller('AddResourceModalCtrl',function($scope,$modalInstance,notify,$route,$http,$log){
    $scope.resource = {};
    $scope.disableBtn=false;
    $scope.submit = function(){
        $scope.disableBtn=true;
        var postData = {"name":$scope.resource.name,
                        "authMeta":$scope.resource.authMeta,
                        "serviceTpl":$scope.resource.serviceTpl,
                        "instanceTpl":$scope.resource.instanceTpl
                    };
        $http({
            method:"POST",
            url:'/ajax/resource/create', 
            data:postData,
            headers:{'Content-Type': 'application/x-www-form-urlencoded'},
            transformRequest: function(obj) {
              var str = [];
              for(var p in obj){
                 str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
              }
              return str.join("&");
            }
          })
          .success(function(data){
              if(data.status == 0 ){
                  notify({ message:'添加资源成功'} );
                  $modalInstance.dismiss('cancel');
                  $route.reload();
              }else{
                   notify({ message:'添加资源失败',classes:"alert-danger"} );
              }
          })
          .error(function(data){
              $log.info(data);
          });
    };
    $scope.cancel = function () {
        $modalInstance.dismiss('cancel');
    };
});