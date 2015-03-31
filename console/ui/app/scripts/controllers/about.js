'use strict';

/**
 * @ngdoc function
 * @name console.controller:console
 * @description
 * # AboutCtrl
 * Controller of the console
 */
angular.module('galaxy.ui.ctrl')
  .controller('AboutCtrl', function ($scope) {
  	console.log("AboutCtrl ");
    $scope.awesomeThings = [
      'HTML5 Boilerplate',
      'AngularJS',
      'Karma'
    ];
  });
