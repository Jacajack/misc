function Pandoc(doc)
  local inc_heading_level = 0

  if not doc.meta.title then
    for i, block in ipairs(doc.blocks) do
      if block.t == "Header" and block.level == 1 then
        doc.meta.title = block.content
		doc.blocks:remove(i)
        break
      end
    end
  end

  for i, block in ipairs(doc.blocks) do
    if block.t == "Header" then
      block.level = block.level + inc_heading_level
    end
      
    if block.t == "Para" then
      for j, inline in ipairs(block.content) do
        if inline.t == "Str" then
          -- Look for the custom tag "{{sec_lvl:<n>}}"
          local content = inline.text
          local tag_regex = "{{sec_lvl:(.-)}}"
          local start_pos, end_pos, match = content:find(tag_regex)

          if match then
			block.content:remove(j)
            inc_heading_level = tonumber(match)
          end
        end
      end
    end
  end
  return doc
end

