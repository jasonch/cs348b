class User < ActiveRecord::Base
  # attr_accessible :title, :body
  has_many :revisions

  validates :login, :presence => true, :uniqueness => true

  ## This doesn't work for some reason, figure out later
  #after_create :add_default_revision
  private
  def add_default_revision
    rev = Revision.new
    rev.title = "New Revision"
    rev.user = self
    rev.save
    self.cur_revision = rev.id
    self.save
  end
end
