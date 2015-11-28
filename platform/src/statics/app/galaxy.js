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
    'easypiechart',
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
      .when('/cluster/status', {
        templateUrl: 'views/cluster.html',
        controller: 'ClusterCtrl'
      })
      .when('/quota', {
        templateUrl: 'views/quota.html',
        controller: 'QuotaCtrl'
      })
      .when('/cluster/tag',{
        templateUrl: 'views/tag.html',
        controller: 'TagPageCtrl'
       })
      .when('/setup', {
        templateUrl: 'views/setup.html',
        controller: 'SetupCtrl'
      })

      .otherwise({
        redirectTo: '/'
      });
  })
  .controller('AppCtrl',['$scope','$rootScope','$http','$location','notify','config',function($scope,$rootScope,$http,$location,notify,config){
      $scope.user = config.user;
      notify.config({duration:1000,templateUrl:'views/notify/notice.html'});
  }]);


}(angular));

