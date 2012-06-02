require 'base64'

class EditorController < ApplicationController
  respond_to :html, :js
  before_filter :set_user_from_session

  @user
  @revision 

  def set_user_from_session
          if (@user.nil?)
                  @user = User.find(session[:user])
          end
  end

  def index
    @user = User.find(session[:user])
    if (params[:id] && params[:id] != "0")
            @revision = Revision.find(params[:id])
            @user.cur_revision = @revision.id
    end
    @revision = Revision.find(@user.cur_revision)

  end

  def commit

    # check input
    if (params[:title].nil? || params[:title] == "" || params[:layers].nil? || params[:layers] == "") 
      flash[:error] = "Error while commiting."
      respond_to do |format| 
            format.js { render :layout => false }
      end 
      return
    end

    
    layers = ActiveSupport::JSON.decode(params[:layers])

    # create the new revision
    rev = Revision.new
    rev.title = params[:title]
    rev.user = @user
    if (@revision) # set parent revision
      rev.revision_id = @revision.id
    end
    rev.save!

    # save each layer 
    layers.each do |layer| 
      if (layer["id"] == "backCanvas")
        name = "thumb"
      else 
        name = layer["name"]
      end
      
      lr = Layer.new
      lr.datapath = layer["filepath"]
      lr.revision = rev
      lr.name = name
      lr.zorder = layer["zorder"]
      lr.save
    end

    # save again here because we didn't know the rev id?
    rev.save!
   
    # update current revision
    @user.cur_revision = rev.id
    @revision = rev 

    flash[:notice] = "Commit successful"
    respond_to do |format| 
            format.js { render :layout => false }
    end 

  end

  def viewTree 

  end

  def getJSONTree 
    @json = {:name => "Root", :children => []}
    # find all the top-level revisoins, then get their children
    Revision.where("revision_id IS NULL AND user_id = #{@user.id}").each do |rev|
      @json[:children] << revisionToJSON(rev)
    end

    respond_with @json do |format|
      format.json {render :layout => false, :text => @json.to_json }
    end    

  end

  def getAllRevisions
    respond_to do |format|
            format.js { render :layout => false, :text => Revision.find_by_user_id(@user.id).to_json }
    end
  end

  def getLayersByRevision
    respond_to do |format|
            format.js {render :layout => false, :text => Revision.find(params[:revId]).layers.to_json}
    end
  end

  ### Private methods ###
  private 
  def revisionToJSON(rev) 
    el = {:name => rev.title, :filepath => rev.layers.select{|l| l.name == 'thumb'}[0].filepath, :id => rev.id, :children => []}
    rev.revisions.each do |r|
      el[:children] << revisionToJSON(r)
    end
    return el
  end

end
