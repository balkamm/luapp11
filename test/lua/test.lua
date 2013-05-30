local function fact(num)
	if(num == 0) then
		return 1
	else
		return num * fact(num-1)
	end
end

return fact(5)