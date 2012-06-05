require 'base64'

class Layer < ActiveRecord::Base
  # attr_accessible :title, :body
  belongs_to :revision
  before_save :write_data_to_file

  def datapath
          @datapath
  end

  def datapath=(path)
          @datapath = path
  end

  def write_data_to_file
      if (self.name == "thumb")
        fullpath = "/images/" + self.revision.id.to_s + "-thumb.png"
      else 
        fullpath = "/images/" + self.revision.id.to_s + "-" + self.zorder.to_s + ".png"
      end
      
      File.open("public" + fullpath, "wb") do |f| 
              f.write(Base64.decode64(@datapath))
      end

      self.filepath = fullpath
  end

  def default
    self.name = "Layer 1"
    self.zorder = 1
    self
  end
end
