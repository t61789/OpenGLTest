import ctypes
import toml
import os
import dearpygui.dearpygui as dpg

try:
    ctypes.windll.shcore.SetProcessDpiAwareness(1)  # 1 = 系统DPI感知
except:
    pass  # 非Windows系统或旧版本Windows会跳过


class TreeNode:

    def __init__(self, comp, parent):
        self.comp = comp
        self.parent = parent
        self.children = []
        self.label = None
        if comp:
            self.label = dpg.get_item_configuration(comp)["label"]


class ControlKeys:
    
    def __init__(self):
        self.root = TreeNode(None, None)
        self.cur_node = self.root
        self.bounds = []
        self.alt_pressed = False

        with dpg.handler_registry():
            dpg.add_key_press_handler(dpg.mvKey_Escape, callback=self._esc_pressed)
            dpg.add_key_press_handler(dpg.mvKey_LAlt, callback=self._alt_pressed)
            dpg.add_key_release_handler(dpg.mvKey_LAlt, callback=self._alt_released)


    def bind(self, node):
        self._unbind()
        
        for i, child in enumerate(node.children):
            dpg.configure_item(child.comp, label=f"[{i+1}]{child.label}")
            self.bounds.append(child)
                
    
    def _unbind(self):
        for node in self.bounds:
            dpg.configure_item(node.comp, label=node.label)
        self.bounds.clear()
        
        
    def _alt_pressed(self):
        if self.alt_pressed:
            return
        
        if dpg.is_key_down(dpg.mvKey_Tilde):
            self.alt_pressed = True
            self._esc_pressed()
            return
        
        for i, child in enumerate(self.cur_node.children):
            key = getattr(dpg, f"mvKey_{i+1}")
            if not dpg.is_key_down(key):
                continue
                
            t = dpg.get_item_type(child.comp)
            if t == "mvAppItemType::mvTab":
                dpg.set_value(dpg.get_item_parent(child.comp), child.comp)
            else:
                cb = dpg.get_item_callback(child.comp)
                if cb:
                    cb()

            self.alt_pressed = True
            if len(child.children) == 0:
                continue

            self._set_cur_node(child)
            break
            
    def _alt_released(self):
        self.alt_pressed = False

        
    def _esc_pressed(self):
        if self.cur_node == self.root:
            return
        
        self.cur_node = self.cur_node.parent
        self.bind(self.cur_node)
        
        
    def _set_cur_node(self, node):
        self.cur_node = node
        self.bind(self.cur_node)
        

class Files:

    def __init__(self, data, parent, parent_node):
        self.collapse_header = dpg.add_collapsing_header(label="Files", parent=parent)
        self.child_window = dpg.add_child_window(parent=self.collapse_header, width=-1, height=-1)
        self.parent = self.child_window

        self.table = dpg.add_table(header_row=False, policy=dpg.mvTable_SizingStretchProp, parent=self.parent,
                                   borders_innerV=True)
        for i in range(3):
            dpg.add_table_column(parent=self.table)
        for d in data:
            self.add_row(d["enable"], d["path"])
        # self.btn_group = dpg.add_group(horizontal=True, parent=self.parent, width=-1)

        btn = dpg.add_button(label="+", callback=self.add_new_row, parent=self.parent, width=-1)
        btn_node = TreeNode(btn, parent_node)
        parent_node.children.append(btn_node)
        
        # dpg.add_button(label="ss", callback=self.get_data, parent=self.btn_group, width=40)
        # dpg.add_button(label="-", callback=self.del_column, parent=self.btn_group, width=40, height=30)
        # dpg.add_button(label="显示", callback=self.get_data, parent=self.btn_group, width=40, height=30)


    def get_data(self):
        data = []
        for row in dpg.get_item_children(self.table, slot=1):
            columns = dpg.get_item_children(row, slot=1)
            data.append({
                "enable": dpg.get_value(columns[0]),
                "path": dpg.get_value(columns[1])
            })

        print(data)
        return data


    def add_new_row(self):
        self.add_row(False, "")


    def add_row(self, enable, path):
        row = dpg.add_table_row(parent=self.table)
        dpg.add_checkbox(default_value=enable, parent=row)
        dpg.add_input_text(default_value=path, parent=row, width=-1)
        dpg.add_button(label="X", callback=lambda: dpg.delete_item(row), parent=row)
        

class Tabs:

    def __init__(self, parent, parent_node):
        self.tab_bar = dpg.add_tab_bar(parent=parent)
        self.convert_pse_tab = dpg.add_tab(label="CovertPse", parent=self.tab_bar)
        self.project_settings_tab = dpg.add_tab(label="ProjectSettings", parent=self.tab_bar)
        
        self.convert_pse_tab_node = TreeNode(self.convert_pse_tab, parent_node) 
        parent_node.children.append(self.convert_pse_tab_node)
        self.project_settings_tab_node = TreeNode(self.project_settings_tab, parent_node)
        parent_node.children.append(self.project_settings_tab_node)
        

class App:
    CONFIG_FILE = "config_test.toml"
    FONT_FILE = "fonts/msyh.ttc"


    def __init__(self):
        self.config = self._load_config()


    def start(self):
        dpg.create_context()
        self._set_font()
        dpg.create_viewport(title='配置文件编辑器', width=600, height=400)

        window = dpg.add_window(tag="Main Window", label="主窗口")
        data = [
            {"enable": True, "path": "123"},
            {"enable": False, "path": "456"},
            {"enable": False, "path": "789"},
            {"enable": True, "path": "10101"},
        ]
        control_keys = ControlKeys()
        tabs = Tabs(window, control_keys.root)
        files = Files(data, tabs.convert_pse_tab, tabs.convert_pse_tab_node)
        control_keys.bind(control_keys.root)

        # 设置主窗口属性
        dpg.setup_dearpygui()
        dpg.show_viewport()
        dpg.set_primary_window("Main Window", True)

        # 主循环
        while dpg.is_dearpygui_running():
            dpg.render_dearpygui_frame()

        dpg.destroy_context()


    @staticmethod
    def _load_config():
        if os.path.exists(App.CONFIG_FILE):
            with open(App.CONFIG_FILE, 'r', encoding="utf-8") as f:
                return toml.load(f)
        return {}


    @staticmethod
    def _save_config(config):
        with open(App.CONFIG_FILE, 'w', encoding="utf-8") as f:
            toml.dump(config, f)


    @staticmethod
    def _set_font():
        with dpg.font_registry():
            with dpg.font(App.FONT_FILE, 18) as font1:  # 增加中文编码范围，防止问号
                dpg.add_font_range_hint(dpg.mvFontRangeHint_Default)
                dpg.add_font_range_hint(dpg.mvFontRangeHint_Chinese_Simplified_Common)
                dpg.add_font_range_hint(dpg.mvFontRangeHint_Chinese_Full)
            dpg.bind_font(font1)

if __name__ == "__main__":
    app = App()
    app.start()