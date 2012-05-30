require 'base64'

class EditorController < ApplicationController
  respond_to :html, :js

  def index
    if (params[:id])
            if (params[:id] == "0")
                    session[:curRev] = nil
                    @revision = nil 
            elsif (!Revision.find_by_id(params[:id]))
              redirect_to :action => "index", :id => 0
            else
              session[:curRev] = params[:id]
            end
    end

    if (!session[:curRev].nil? && session[:curRev] != "0")
            @revision = Revision.find(session[:curRev])
            logger.debug "Session curRev: #{session[:curRev]}"
    else
            logger.debug "No CurRev in session"
    end

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

    if (!session[:curRev].nil?) 
            @revision = Revision.find(session[:curRev])
    end
    
    layers = ActiveSupport::JSON.decode(params[:layers])

    # create the new revision
    rev = Revision.new
    rev.title = params[:title]
    if (@revision)
      rev.revision_id = @revision.id
    end
    rev.save

    # save each layer 
    layers.each do |layer| 
      id = layer["id"]
      z = layer["zorder"]
      if (id == "backCanvas")
        name = "thumb"
        fullpath = "/images/" + rev.id.to_s + "-thumb.png"
      else 
        name = layer["name"]
        fullpath = "/images/" + rev.id.to_s + "-" + z + ".png"
      end
      
      File.open("public" + fullpath, "wb") do |f| 
              f.write(Base64.decode64(layer["filepath"]))
      end

      lr = Layer.new
      lr.filepath = fullpath
      lr.revision = rev
      lr.name = name
      lr.zorder = z
      lr.save
    end

    # save again here because we didn't know the rev id?
    rev.save
   
    # update current revision
    session[:curRev] = rev.id
    @revision = rev 

    flash[:message] = "Commit successful"
    respond_to do |format| 
            format.js { render :layout => false }
    end 

  end

  def viewTree 

  end

  def getJSONTree 
    @json = {:name => "Root", :children => []}
    # find all the top-level revisoins, then get their children
    Revision.where("revision_id IS NULL").each do |rev|
      @json[:children] << revisionToJSON(rev)
    end

    respond_with @json do |format|
      format.json {render :layout => false, :text => @json.to_json }
    end    

  end

  def getAllRevisions
    respond_to do |format|
            format.js { render :layout => false, :text => Revision.all.to_json }
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
