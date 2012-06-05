class ApplicationController < ActionController::Base
  protect_from_forgery
  before_filter :require_login 

  def require_login
    if (session[:user].nil?)
      flash[:notice] = "You need to log in to do that"
      session[:redirect_from] = request.referer
      redirect_to :controller => "user", :action => "login"
    end
  end

end
