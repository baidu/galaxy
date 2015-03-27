(function(angular){
    angular.module('galaxy.ui.ctrl')
           .controller('ServiceOnlineCtrl',function($scope,$http,$routeParams,notify,$location){
             $scope.serviceName = $routeParams.serviceName;
             $scope.showAlert = false;
             $scope.formData = {count:0,meta:"",name:$routeParams.serviceName,editable:false};
             $scope.submitOnlineReq = function(){
                   $http({  method:"POST",
                            url:"/ajax/instance/online",
                            data:$scope.formData ,
                            headers:{'Content-Type': 'application/x-www-form-urlencoded'},
                            transformRequest: function(obj) {
                                      var str = [];
                                      for(var p in obj){
                                         str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
                                      }
                                      return str.join("&");
                             }
                          }).success(function(data){
                            if(data.status == 0 ){
                              notify({ message:'操作成功'} );
                              $location.path("/service/"+$scope.serviceName+"/status");
                            }else{
                               notify({ message:'操作失败：'+data.msg,classes:"alert-danger"} );
                            }
                            
                          }).error(function(data){
                          })
               };
            $scope.onlineStatus = function(){
              $http.get("/ajax/instance/onlineStatus?name="+$scope.serviceName)
                   .success(function(data){
                     if(data.status == 0 ){
                       if(data.data.form != null && data.group1Count!=0){
                          $scope.formData.meta = data.data.form.formData;
                          $scope.showAlert = true;
                       }
                       $scope.onlineStatus = data.data;
                     }
                   })
                   .error(function(){

                   })
            };
            $scope.onlineStatus();

           });
}(angular))