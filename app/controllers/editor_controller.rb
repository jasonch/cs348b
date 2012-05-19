require 'base64'

class EditorController < ApplicationController
  respond_to :html, :js

  def index
    if (params[:id])
            if (params[:id] == "0")
                    session[:curRev] = nil;
                    @revision = nil;
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
    if (params[:title].nil? || params[:title] == "" || params[:filepath].nil?) 
      flash[:error] = "Error while commiting."
      respond_to do |format| 
            format.js { render :layout => false }
      end 
      return
    end

    if (!session[:curRev].nil?) 
            @revision = Revision.find(session[:curRev])
    end
    
    filepath = params[:filepath]

    # create the new revision
    rev = Revision.new
    rev.title = params[:title]
    rev.body = ""
    if (@revision)
      rev.revision_id = @revision.id
    end
    rev.save

    # save again here because we don't know the rev id until it's saved first
    fullpath = "/images/" + params[:title] + "-" + rev.id.to_s + ".png"
    File.open("public" + fullpath, "wb") do |f| 
            f.write(Base64.decode64(filepath))
    end
    rev.filepath = fullpath
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



  ### Private methods ###
  private 
  def revisionToJSON(rev) 
    el = {:name => rev.title, :body => rev.body, :filepath => rev.filepath, :id => rev.id, :children => []}
    rev.revisions.each do |r|
      el[:children] << revisionToJSON(r)
    end
    return el
  end

end
