(function(angular){
    angular.module( 'galaxy.ui.promot', [])
           .controller('PromotCtrl',function($scope,$modalInstance,message){
               $scope.message = message;
               $scope.sure = function(){
                   $modalInstance.close(true);
               }
               $scope.no = function(){
                   $modalInstance.close(false);
               }
           });
}(angular))