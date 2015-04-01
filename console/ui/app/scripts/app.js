'use strict';


(function(angular){


/**
 * @ngdoc overview
 * @name uidemoApp
 * @description
 * # uidemoApp
 *
 * Main module of the application.
 */
var galaxy = angular.module('galaxy.ui', [
    'ngAnimate',
    'ngCookies',
    'ngResource',
    'ngRoute',
    'ngSanitize',
    'galaxy.ui.treeview',
    'ui.bootstrap',
    'galaxy.ui.modal.service',
    'galaxy.ui.ctrl',
    'galaxy.ui.loader',
    'galaxy.ui.breadcrumb',
    'galaxy.ui.promot',
    'cgNotify',
    'ui.select'
  ])
  .config(function ($routeProvider) {
    $routeProvider
      .when('/', {
        templateUrl: 'views/main.html',
        controller: 'HomeCtrl'
      })
      .when('/service', {
        templateUrl: 'views/main.html',
        controller: 'HomeCtrl'
      })
      .when('/cluster', {
        templateUrl: 'views/cluster.html',
        controller: 'ClusterCtrl'
      })
      .otherwise({
        redirectTo: '/'
      });
  })
  .controller('AppCtrl',['$scope','$rootScope','$http','$location','notify',function($scope,$rootScope,$http,$location,notify){
      notify.config({duration:1000,templateUrl:'views/notify/notice.html'});
  }]);

  function fetchConfig(){
    //var masterAddr = prompt("输入galaxy master地址");
    var masterAddr = 'cp01-rdqa-dev400.cp01.baidu.com:8102';
    var initInjector = angular.injector(["ng"]);
    var $http = initInjector.get("$http");
    return $http.get("/conf/get").then(function(response) {
            galaxy.constant("config", {config:response.data.data.config,masterAddr:masterAddr,home:response.data.data.home,service:response.data.data.service});
        }, function(errorResponse) {

     });
  }
  fetchConfig().then(function(){
     angular.element(document).ready(function() {
            angular.bootstrap(angular.element('body'), ["galaxy.ui"]);
      });
  });


}(angular));
