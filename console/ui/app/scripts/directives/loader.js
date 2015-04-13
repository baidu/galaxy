(function(angular){

angular.module( 'galaxy.ui.loader', [] ).directive( 'routerLoader',
        ['$compile','$rootScope','$location',function( $compile ,$rootScope,$location) {
            return {
                restrict: 'A',
                link: function ( $scope, element, attrs ) {
                    $scope.showloader =false;
                    var listenerStart = $rootScope.$on("$routeChangeStart", function () {
                                $scope.showloader =true;
                            });            
                    var listenerEnd = $rootScope.$on("$routeChangeSuccess", function () {
                                $scope.showloader =false;
                            });            
                    $scope.$on('$destroy',listenerStart);
                    $scope.$on('$destroy',listenerEnd);
                }}
            
        }]);

}(angular));


