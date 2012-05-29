class Layer < ActiveRecord::Base
  # attr_accessible :title, :body
  belongs_to :revision

  def default
    self.name = "Layer 1"
    self.zorder = 1
    self
  end
end
