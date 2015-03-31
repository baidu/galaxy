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
      .when('/resource', {
        templateUrl: 'views/resource.html',
        controller: 'ResourceCtrl'
      })
      .when('/login', {
        templateUrl: 'views/login.html',
        controller: 'LoginCtrl'
      })
      .when('/service/:serviceName/', {
        templateUrl: 'views/instance.html',
        controller: 'ServiceCtrl'
      })
      .when('/service/:serviceName/status', {
        templateUrl: 'views/instance.html',
        controller: 'ServiceCtrl'
      })
      .when('/service/:serviceName/online', {
        templateUrl: 'views/online.tpl',
        controller: 'ServiceOnlineCtrl'
      })
      .otherwise({
        redirectTo: '/'
      });
  })
  .controller('AppCtrl',['$scope','$rootScope','$http','$location','notify',function($scope,$rootScope,$http,$location,notify){
      notify.config({duration:1000,templateUrl:'views/notify/notice.html'});
  }]);

  function fetchConfig(){
    var initInjector = angular.injector(["ng"]);
    var $http = initInjector.get("$http");
    return $http.get("/ajax/config/get").then(function(response) {
            galaxy.constant("config", {config:response.data.config,home:JSON.parse(response.data.home),service:JSON.parse(response.data.service)});
        }, function(errorResponse) {

     });
  }
  fetchConfig().then(function(){
     angular.element(document).ready(function() {
            angular.bootstrap(angular.element('body'), ["galaxy.ui"]);
      });
  });


}(angular));
