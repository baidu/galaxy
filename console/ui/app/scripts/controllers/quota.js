'use strict';
// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

(function(angular){


angular.module('galaxy.ui.ctrl').controller('QuotaCtrl',function($scope,
                                                                $http,
                                                                $log,
                                                                $interval,
                                                                config){

   $scope.getStat = function(){
      $http.get(config.rootPrefixPath + "quota/info")
           .success(function(data){
               if(data.status == 0 ){
                  $scope.statList = data.data;
               }
          })
           .error(function(err){
             $log.error('fail to get service '+ $routeParams.serviceName+' status');
           });
    }
    $scope.getStat();
});



}(angular));
