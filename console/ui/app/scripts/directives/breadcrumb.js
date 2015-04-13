(function(angular){

var NavBreadcrumb = function($scope,$location){
    this.$scope = $scope;
    this.$location = $location;
}
NavBreadcrumb.prototype.parsePath = function(hash){
    if(!hash){
          return [];
        }
    var pathArray = [];
    angular.forEach(hash.split('/'),function(value,key){
          var trimedValue = value.trim();
          if(trimedValue){
            this.push(trimedValue);
          }
    },pathArray);
    return pathArray;
}
NavBreadcrumb.prototype.update = function(){
    var pathArray = this.parsePath(this.$location.path());
     this.$scope.mybreadcrumb='';
    //service path
    if(pathArray.length > 1){
        this.$scope.mybreadcrumb='mybreadcrumb';
    }
}

angular.module( 'galaxy.ui.breadcrumb', [] );
angular.module( 'galaxy.ui.breadcrumb' ).directive( 'navBreadcrumb',
        [function( ) {
            return {
                restrict: 'A',
                templateUrl:"views/breadcrumb.html",
                controller: 'navBreadcrumbCtrl'}
   
            
 }]);
angular.module( 'galaxy.ui.breadcrumb').controller('navBreadcrumbCtrl',['$scope','$rootScope','$location',
    function($scope,$rootScope,$location){
        $scope.breadcrumbList = [];
        var navBreadcrumb = new NavBreadcrumb($scope,$location);
        var listener = $rootScope.$on("$routeChangeSuccess", function () {
                  navBreadcrumb.update();
            });
        $scope.$on('$destroy',listener);
    }

 ]);
angular.module( 'galaxy.ui.breadcrumb').filter('safeHtml', ['$sce', function($sce){
        return function(text) {
            return $sce.trustAsHtml(text);
        };
  }]);

}(angular));

