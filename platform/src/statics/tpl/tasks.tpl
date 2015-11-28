 
<div class="ui celled divided list">
 {{#logs}} 
  <div class="item" >
    <i class="alarm middle aligned icon"></i>
    <div class="content">
      {{id}}
      <div class="description">
        {{level}} {{error}} at {{ftime}} 
        <br>agent : {{agent_addr}}
        <br>exit_code : {{exit_code}}
        <br>gc_dir : {{gc_dir}}
        <br>cmd : {{cmd}}
      </div>
    </div>
  </div>
  {{/logs}}
</div>
