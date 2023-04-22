--select firstname, lastname from employees e 
--where exists (select * from orders o where customerid='ALFKI' and e.employeeid=o.employeeid)

select * from orders o where 
	not exists (select * from [order details] od join products p on p.productid=od.productid where productname='Scottish Longbreads' and od.orderid=o.orderid)
	and exists (select * from [order details] od join products p on p.productid=od.productid where productname='Chocolade' and od.orderid=o.orderid)
