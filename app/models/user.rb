class User < ActiveRecord::Base
  # attr_accessible :title, :body
  has_many :revisions

  validates :login, :presence => true, :uniqueness => true

  after_create :add_default_revision

  ## This doesn't work for some reason, figure out later
  def add_default_revision
    rev = Revision.new
    rev.title = "New Revision"
    rev.user = self
    rev.save
    self.cur_revision = rev.id
    self.save
  end
end
