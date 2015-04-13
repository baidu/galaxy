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
    'ui.bootstrap',
    'galaxy.ui.ctrl',
    'galaxy.ui.breadcrumb',
    'galaxy.ui.promot',
    'cgNotify',
    'galaxy.ui.loader',
    'galaxy.ui.treeview',
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
      .when('/setup', {
        templateUrl: 'views/setup.html',
        controller: 'SetupCtrl'
      })

      .otherwise({
        redirectTo: '/'
      });
  })
  .controller('AppCtrl',['$scope','$rootScope','$http','$location','notify',function($scope,$rootScope,$http,$location,notify){
      notify.config({duration:1000,templateUrl:'views/notify/notice.html'});
  }]);

  function fetchConfig(){
    var initInjector = angular.injector(["ng","ngCookies"]);
    var $http = initInjector.get("$http");
    var $cookies = initInjector.get('$cookies');
    var masterAddr = null ;
    if($cookies.masterAddr != undefined ){
        masterAddr = $cookies.masterAddr;
    }
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
