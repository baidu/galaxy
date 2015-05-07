'use strict';
// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com
// Date  : 2015-03-31
(function(angular){
angular.module('galaxy.ui.ctrl')
       .controller('ClusterCtrl',function($scope,
                                          $modal,
                                          $http,
                                          $routeParams,
                                          $log,
                                          notify,
                                          config,
                                          $location){
           if(config.masterAddr == null ){
              $location.path('/setup');
              return;
           }
          $scope.listTaskByAgent = function(agent){
                 var modalInstance = $modal.open({
                 templateUrl: 'views/task.html',
                 controller: 'TaskForAgentCtrl',
                 keyboard:false,
                 size:'lg',
                 backdrop:'static',
                 resolve:{
                    agent:function(){
                      return agent;
                 }
             }
           });
          }  


           $scope.machineList = [];
           $http.get("/cluster/status?master="+config.masterAddr)
                .success(function(data){
                    if(data.status == 0){
                        $scope.machineList = data.data.machinelist;
                        $scope.total_node_num = data.data.total_node_num;
                        $scope.total_cpu_num = data.data.total_cpu_num;
                        $scope.total_cpu_used = data.data.total_cpu_used;
                        $scope.total_mem_used = data.data.total_mem_used;
                        $scope.total_mem_num = data.data.total_mem_num;
                        $scope.total_task_num = data.data.total_task_num;
                        $scope.total_mem_real_used = data.data.total_mem_real_used;
                        $scope.total_cpu_real_used = data.data.total_cpu_real_used;
                    }else{
                    
                    }
                })
                .error(function(){})
    
}
)
 
}(angular));
