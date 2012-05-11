class EditorController < ApplicationController
  respond_to :html, :js
  @notice

  def index
    if (!session[:curRev].nil?)
            @revision = Revision.find(session[:curRev])
            logger.debug "Session curRev: #{session[:curRev]}"
    else
            logger.debug "No CurRev in session"
    end

  end

  def commit
    if (!session[:curRev].nil?) 
            @revision = Revision.find(session[:curRev])
    end

    # check if no changes made
    if (@revision && params[:body] == @revision.body && params[:title] == @revision.title)
      @notice = 'No change was made'
      respond_to do |format| 
        format.html { redirect_to 'index', :notice => 'No change made' }
      end
      return
    end

    # check for empty revisions
    if (params[:title] == "" or params[:body] == "")
            @notice = 'No data'
            respond_to do |format|
                    format.html { redirect_to 'index', :notice => 'No data' }
            end
            return
    end

    # create new revision
    revision = Revision.new
    revision.body = params[:body]
    revision.title = params[:title]
    # if this revision would be a child revision
    if (defined? @revision)
            revision.revision_id = @revision.id
    end
    revision.save()
    @revision = revision
    session[:curRev] = @revision.id

    @notice = 'Commit saved'
    respond_to do |format| 
        format.html { redirect_to 'index', :notice => "Committed" } 
        format.js { render :layout => false } 
    end
  end


  def viewTree 

  end

  def getJSONTree 
    @json = {:name => "Root", :children => []}
    # find all the top-level revisoins, then get their children
    Revision.where("revision_id IS NOT NULL").each do |rev|
      @json[:children] << revisionToJSON(rev)
    end

    respond_with @json do |format|
            format.json {render :layout => false, :text => @json.to_json }

    end    

  end



  ### Private methods ###
  private 
  def revisionToJSON(rev) 
    el = {:name => rev.title, :body => rev.body, :children => []}
    rev.revisions.each do |r|
      el[:children] << revisionToJSON(r)
    end
    return el
  end

end
