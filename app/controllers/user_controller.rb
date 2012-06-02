class UserController < ApplicationController
  skip_before_filter :require_login
  def login
    if (session[:user])
            reset_session
    end
  end

  def post_login
    login = params[:user][:login] 

    if (!User.find_by_login(login).nil?)
      session[:user] = User.find_by_login(login).id
    else # just create a new user if login doesn't already exist
      user = User.new(params[:user])
      if (!user.save)
              flash[:notice] = user.errors.full_messages[0]
              redirect_to :action => "login"
              return
      end
      flash[:notice] = "New User created with login #{login}"
      session[:user] = user.id
    end

    #if (session[:redirect_from].nil?)
      redirect_to :controller => "editor", :action => "index"
    #else
    #  flash[:notice] = "Redirecting back to referer"
    #  redirect_to session[:redirect_from]
    #end
  end


  def logout
    reset_session
    redirect_to :action => "login"
  end
end
