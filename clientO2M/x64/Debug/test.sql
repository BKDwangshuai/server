use unity;
drop procedure if exists 产品喷印;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `产品喷印`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 20;end$$
DELIMITER ;

drop procedure if exists 产品检测;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `产品检测`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 19;end$$
DELIMITER ;

drop procedure if exists 关回收检测;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `关回收检测`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 10;end$$
DELIMITER ;

drop procedure if exists 关闭墨线;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `关闭墨线`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 11;end$$
DELIMITER ;

drop procedure if exists 列点数;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `列点数`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=14); end$$
DELIMITER ;

drop procedure if exists 压力;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `压力`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=1); end$$
DELIMITER ;

drop procedure if exists 喷头温度;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `喷头温度`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=4); end$$
DELIMITER ;

drop procedure if exists 墨水时间;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `墨水时间`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=12); end$$
DELIMITER ;

drop procedure if exists 墨水液位;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `墨水液位`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=5); end$$
DELIMITER ;

drop procedure if exists 墨水温度;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `墨水温度`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=3); end$$
DELIMITER ;

drop procedure if exists 实际粘度;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `实际粘度`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=8); end$$
DELIMITER ;

drop procedure if exists 开关供墨阀;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开关供墨阀`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 4;end$$
DELIMITER ;

drop procedure if exists 开关冲洗阀;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开关冲洗阀`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 9;end$$
DELIMITER ;

drop procedure if exists 开关喷嘴;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开关喷嘴`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 3;end$$
DELIMITER ;

drop procedure if exists 开关排气阀;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开关排气阀`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 5;end$$
DELIMITER ;

drop procedure if exists 开关机打印;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开关机打印`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 26;end$$
DELIMITER ;

drop procedure if exists 开关泵;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开关泵`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 2;end$$
DELIMITER ;

drop procedure if exists 开关清洗阀;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开关清洗阀`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 6;end$$
DELIMITER ;

drop procedure if exists 开关溶剂阀;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开关溶剂阀`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 7;end$$
DELIMITER ;

drop procedure if exists 开关粘度阀;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开关粘度阀`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 8;end$$
DELIMITER ;

drop procedure if exists 开冲洗喷嘴;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开冲洗喷嘴`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 14;end$$
DELIMITER ;

drop procedure if exists 开反吸喷嘴;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开反吸喷嘴`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 15;end$$
DELIMITER ;

drop procedure if exists 开墨线校准;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开墨线校准`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 16;end$$
DELIMITER ;

drop procedure if exists 开墨路循环;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开墨路循环`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 17;end$$
DELIMITER ;

drop procedure if exists 开测试粘度;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开测试粘度`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 13;end$$
DELIMITER ;

drop procedure if exists 开添加溶剂;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `开添加溶剂`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 12;end$$
DELIMITER ;

drop procedure if exists 打印字节;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `打印字节`(IN `eid` INT)
begin select cprint from code where cid in (select max(cid) from code where equipment_id=eid); end$$
DELIMITER ;

drop procedure if exists 泵速;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `泵速`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=2); end$$
DELIMITER ;

drop procedure if exists 泵速或压力模式;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `泵速或压力模式`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 1;end$$
DELIMITER ;

drop procedure if exists 溶剂液位;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `溶剂液位`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=6); end$$
DELIMITER ;

drop procedure if exists 目标粘度;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `目标粘度`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=7); end$$
DELIMITER ;

drop procedure if exists 相位;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `相位`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=11); end$$
DELIMITER ;

drop procedure if exists 编码器频率;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `编码器频率`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=13); end$$
DELIMITER ;

drop procedure if exists 计数器;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `计数器`(IN `eid` INT)
begin
	declare n int default 0;
	declare num int default 0;
	declare cnl int default 0;
	declare tempvalue int default 0;
	select tvalue into num from tempdata where equipment_id = eid and tcnl = 21;
	drop table if exists tmp_table;
	CREATE TEMPORARY TABLE tmp_table(   
	value int
	);
	insert tmp_table(value) values (num);
	while n<num DO
		set cnl=22+n;
		select tvalue into tempvalue from tempdata where equipment_id = eid and tcnl = cnl;
		insert tmp_table(value) values (tempvalue);
		set n = n+1;
	end while;
	select * from tmp_table;
end$$
DELIMITER ;

drop procedure if exists 调制电压;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `调制电压`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=10); end$$
DELIMITER ;

drop procedure if exists 高压;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `高压`(in eid int)
begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=9); end$$
DELIMITER ;

drop procedure if exists 高压开关;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `高压开关`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 18;end$$
DELIMITER ;

drop procedure if exists 电眼电平;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `电眼电平`(in eid int)
begin select tvalue from tempdata where equipment_id = eid and tcnl = 27;end$$
DELIMITER ;

drop procedure if exists 设备属性;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `设备属性`(in e_id int)
begin
	DECLARE equip_type VARCHAR(100);
	select etype into equip_type from equipment where eid = e_id;
	select ue_property from unity_equip where ue_id=equip_type;
end$$
DELIMITER ;

drop procedure if exists readMysql;
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `readMysql`(in eid int,in proName VARCHAR(100))
begin
	CASE proName
		WHEN 'Pressure' THEN
		CALL 压力(eid);
		WHEN 'Pump speed' THEN
		CALL 泵速(eid);
		WHEN 'Ink temperature' THEN
		CALL 墨水温度(eid);
		WHEN 'Nozzle temperature' THEN
		CALL 喷头温度(eid);
		WHEN 'Ink level' THEN
		CALL 墨水液位(eid);
		WHEN 'Solvent level' THEN
		CALL 溶剂液位(eid);
		WHEN 'Target viscosity' THEN
		CALL 目标粘度(eid);		
		WHEN 'Actual viscosity' THEN
		CALL 实际粘度(eid);
		WHEN 'High voltage' THEN
		CALL 高压(eid);
		WHEN 'Modulation voltage' THEN
		CALL 调制电压(eid);
		WHEN 'Phase' THEN
		CALL 相位(eid);		
		WHEN 'Ink time' THEN
		CALL 墨水时间(eid);
		WHEN 'Encoder frequency' THEN
		CALL 编码器频率(eid);
		WHEN 'Number of columns' THEN
		CALL 列点数(eid);
		WHEN 'Code byte' THEN
		CALL 打印字节(eid);
		
		WHEN 'Pump speed or pressure mode' THEN
		CALL 泵速或压力模式(eid);
		WHEN 'Switch pump' THEN
		CALL 开关泵(eid);
		WHEN 'Switch nozzle' THEN
		CALL 开关喷嘴(eid);
		WHEN 'Ink supply valve' THEN
		CALL 开关供墨阀(eid);
		WHEN 'Vent valve' THEN
		CALL 开关排气阀(eid);
		WHEN 'Cleaning valve' THEN
		CALL 开关清洗阀(eid);
		WHEN 'Solvent valve' THEN
		CALL 开关溶剂阀(eid);
		WHEN 'Viscosity valve' THEN
		CALL 开关粘度阀(eid);
		WHEN 'Flush valve' THEN
		CALL 开关冲洗阀(eid);
		WHEN 'Close recycling detection' THEN
		CALL 关回收检测(eid);
		WHEN 'Close ink line' THEN
		CALL 关闭墨线(eid);
		WHEN 'Add solvent' THEN
		CALL 开添加溶剂(eid);
		WHEN 'Test viscosity' THEN
		CALL 开测试粘度(eid);
		WHEN 'Flushing nozzle' THEN
		CALL 开冲洗喷嘴(eid);
		WHEN 'The reverse suction nozzle' THEN
		CALL 开反吸喷嘴(eid);
		WHEN 'Ink line calibration' THEN
		CALL 开墨线校准(eid);
		WHEN 'Ink road cycle' THEN
		CALL 开墨路循环(eid);
		WHEN 'High voltage switch' THEN
		CALL 高压开关(eid);
		WHEN 'Product testing' THEN
		CALL 产品检测(eid);
		WHEN 'Product spray printing' THEN
		CALL 产品喷印(eid);
		WHEN 'Counter' THEN
		CALL 计数器(eid);
		WHEN 'Switch printing' THEN
		CALL 开关机打印(eid);
		WHEN 'Eye level' THEN
		CALL 电眼电平(eid);
		
		WHEN 'Property' THEN
		CALL 设备属性(eid);
	END CASE;
end$$
DELIMITER ;