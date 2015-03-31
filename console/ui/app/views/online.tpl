<div ><h4>上线操作</h4></div>
<form style="width:60%">
  <div ng-show="showAlert"class="alert alert-warning"role="alert">
    <strong>存在小流量上线</strong><br>
    已经更新实例数：{{onlineStatus.group1Count}}
    未更新实例数：{{onlineStatus.group0Count}}
  </div>
  <div class="form-group" >
    <label for="serviceName">上线实例数<span class="glyphicon glyphicon-question-sign" aria-hidden="true"></label>
    <div class="input-group">
      <input type="number"  min="0" ng-model="formData.count"class="form-control" >
      <span class="input-group-addon">实例数</span>
    </div>
  </div>
  <div class="form-group">
    <label for="instaceMeta">实例配置</label>
    <textarea ng-readonly="showAlert" id="instaceMeta" class="form-control scrollable" rows="15" ng-model="formData.meta" ></textarea>
  </div>
  <button class="btn btn-primary"  ng-click="submitOnlineReq()">提交</button>
</form>









