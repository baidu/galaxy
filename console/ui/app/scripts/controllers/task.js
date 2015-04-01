// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com
// Date  : 2015-03-31

(function(angular){


angular.module('galaxy.ui.ctrl').controller('TaskCtrl',function($scope,
                                                                $modalInstance,
                                                                $http,
                                                                $log,
                                                                service,
                                                                config){

   $scope.getTask = function(){
      $http.get("/taskgroup/status?id="+service.id+"&master="+config.masterAddr)
           .success(function(data){
               if(data.status == 0 ){
                  $scope.tasklist = data.data.taskList;
               }
          })
           .error(function(err){
             $log.error('fail to get service '+ $routeParams.serviceName+' status');
           });
    }
   $scope.getTask();
   $scope.close =function(){
      $modalInstance.dismiss('cancel'); 
   }
   
});

angular.module('galaxy.ui.ctrl').controller('TaskForAgentCtrl',function($scope,
                                                                $modalInstance,
                                                                $http,
                                                                $log,
                                                                agent,
                                                                config){

   $scope.getTask = function(){
      $http.get("/taskgroup/status?agent="+agent.host+"&master="+config.masterAddr)
           .success(function(data){
               if(data.status == 0 ){
                  $scope.tasklist = data.data.taskList;
               }
          })
           .error(function(err){
             $log.error('fail to get service '+ $routeParams.serviceName+' status');
           });
    }
   $scope.getTask();
   $scope.close =function(){
      $modalInstance.dismiss('cancel'); 
   }
   
});




}(angular))
