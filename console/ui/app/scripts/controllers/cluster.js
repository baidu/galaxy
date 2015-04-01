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
                                          config){
           $scope.machineList = [];
           $http.get("/cluster/status?master="+config.masterAddr)
                .success(function(data){
                    if(data.status == 0){
                        $scope.machineList = data.data;
                    }else{
                    
                    }
                })
                .error(function(){})
    
}
)
 
}(angular))
