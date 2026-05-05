<template>
	<view class="page">
		<AppHeader title="系统设置" subtitle="OneNet 平台连接参数" />

		<SectionCard>
			<view class="connect-card">
				<view class="connect-icon">✓</view>
				<view class="connect-main">
					<text class="connect-title">平台连接状态</text>
					<text class="connect-text">{{ connectionText }}</text>
				</view>
				<StatusBadge :text="connectionBadge.text" :type="connectionBadge.type" />
			</view>
		</SectionCard>

		<SectionCard title="平台配置" subtitle="配置会保存到本地 storage">
			<view class="form-list">
				<view class="mode-row">
					<text class="platform-label">数据模式</text>
					<view class="mode-tabs">
						<text
							v-for="mode in dataModes"
							:key="mode.value"
							class="mode-tab"
							:class="{ active: form.dataMode === mode.value }"
							@tap="setDataMode(mode.value)"
						>
							{{ mode.label }}
						</text>
					</view>
				</view>
				<view class="platform-row">
					<text class="platform-label">平台类型</text>
					<view class="platform-value">
						<text>OneNet</text>
					</view>
				</view>
				<SettingField label="Product ID" v-model="form.productId" placeholder="请输入 Product ID" />
				<SettingField label="Token" v-model="form.token" placeholder="请输入 Token" password />
				<SettingField label="Device ID" v-model="form.deviceId" placeholder="请输入 Device ID" />
			</view>
		</SectionCard>

		<SectionCard title="通信参数">
			<view class="form-list">
				<SettingField label="服务器地址" v-model="form.serverHost" placeholder="api.heclouds.com" />
				<SettingField label="数据上报周期" v-model="form.reportInterval" type="number" placeholder="60" />
			</view>
		</SectionCard>

		<view class="actions">
			<button class="action-btn secondary" @tap="handleTest">测试连接</button>
			<button class="action-btn primary" @tap="handleSave">保存配置</button>
		</view>

		<AppTabBar active="settings" />
	</view>
</template>

<script>
import AppHeader from '../../components/AppHeader.vue'
import AppTabBar from '../../components/AppTabBar.vue'
import SectionCard from '../../components/SectionCard.vue'
import SettingField from '../../components/SettingField.vue'
import StatusBadge from '../../components/StatusBadge.vue'
import { loadSettings, saveSettings } from '../../utils/storage.js'
import { testConnection } from '../../utils/onenet.js'

export default {
	components: {
		AppHeader,
		AppTabBar,
		SectionCard,
		SettingField,
		StatusBadge
	},
	data() {
		return {
			form: loadSettings(),
			testing: false,
			dataModes: [
				{ label: 'Mock', value: 'mock' },
				{ label: 'OneNet', value: 'onenet' }
			]
		}
	},
	computed: {
		connectionBadge() {
			if (this.form.connectionStatus === 'connected') {
				return {
					text: '已连接',
					type: 'success'
				}
			}
			if (this.form.connectionStatus === 'failed') {
				return {
					text: '连接失败',
					type: 'danger'
				}
			}
			return {
				text: '未测试',
				type: 'muted'
			}
		},
		connectionText() {
			if (this.form.connectionStatus === 'connected') {
				return '已连接 · 当前平台 OneNet'
			}
			if (this.form.connectionStatus === 'failed') {
				return '连接失败 · 请检查 Token 与设备参数'
			}
			return '未测试 · 当前平台 OneNet'
		}
	},
	onShow() {
		this.form = loadSettings()
	},
	methods: {
		setDataMode(mode) {
			this.form.dataMode = mode
			this.form.connectionStatus = 'untested'
		},
		handleTest() {
			if (this.testing) return
			this.testing = true
			testConnection(this.form)
				.then(() => {
					this.form.connectionStatus = 'connected'
					this.form = saveSettings(this.form)
					uni.showToast({
						title: '连接成功',
						icon: 'success'
					})
					this.testing = false
				})
				.catch((error) => {
					this.form.connectionStatus = 'failed'
					this.form = saveSettings(this.form)
					uni.showToast({
						title: error && error.message ? error.message : '连接失败',
						icon: 'none'
					})
					this.testing = false
				})
		},
		handleSave() {
			this.form = saveSettings(this.form)
			uni.showToast({
				title: '配置已保存',
				icon: 'success'
			})
		}
	}
}
</script>

<style>
.page {
	min-height: 100vh;
	padding: 32rpx 32rpx 168rpx;
	background: #F6F9FC;
	box-sizing: border-box;
	display: flex;
	flex-direction: column;
	gap: 24rpx;
}

.connect-card {
	display: flex;
	align-items: center;
	gap: 20rpx;
}

.connect-icon {
	width: 78rpx;
	height: 78rpx;
	border-radius: 26rpx;
	background: linear-gradient(135deg, #22C55E 0%, #14B8A6 100%);
	color: #FFFFFF;
	display: flex;
	align-items: center;
	justify-content: center;
	font-size: 42rpx;
	font-weight: 900;
	box-shadow: 0 12rpx 24rpx rgba(20, 184, 166, 0.24);
}

.connect-main {
	flex: 1;
	min-width: 0;
}

.connect-title,
.connect-text {
	display: block;
}

.connect-title {
	font-size: 28rpx;
	font-weight: 900;
	color: #0F172A;
}

.connect-text {
	margin-top: 6rpx;
	font-size: 22rpx;
	color: #64748B;
}

.form-list {
	display: flex;
	flex-direction: column;
	gap: 24rpx;
}

.mode-row,
.platform-row {
	display: flex;
	flex-direction: column;
	gap: 12rpx;
}

.platform-label {
	font-size: 24rpx;
	font-weight: 750;
	color: #334155;
}

.platform-value {
	height: 92rpx;
	padding: 0 26rpx;
	border-radius: 24rpx;
	background: #F8FAFC;
	border: 1rpx solid #E2E8F0;
	display: flex;
	align-items: center;
	font-size: 28rpx;
	font-weight: 750;
	color: #0F172A;
	box-sizing: border-box;
}

.mode-tabs {
	padding: 8rpx;
	border-radius: 999rpx;
	background: #EEF4FF;
	display: flex;
	gap: 8rpx;
}

.mode-tab {
	flex: 1;
	height: 70rpx;
	line-height: 70rpx;
	border-radius: 999rpx;
	text-align: center;
	font-size: 26rpx;
	font-weight: 850;
	color: #64748B;
}

.mode-tab.active {
	color: #FFFFFF;
	background: linear-gradient(135deg, #2563EB 0%, #14B8A6 100%);
	box-shadow: 0 8rpx 18rpx rgba(37, 99, 235, 0.18);
}

.actions {
	display: flex;
	gap: 20rpx;
}

.action-btn {
	flex: 1;
	margin: 0;
	padding: 0;
	height: 92rpx;
	line-height: 92rpx;
	border-radius: 999rpx;
	font-size: 28rpx;
	font-weight: 900;
	box-shadow: 0 14rpx 26rpx rgba(15, 23, 42, 0.08);
}

.action-btn::after {
	border: 0;
}

.action-btn.primary {
	background: linear-gradient(135deg, #2563EB 0%, #14B8A6 100%);
	color: #FFFFFF;
}

.action-btn.secondary {
	background: #FFFFFF;
	color: #2563EB;
	border: 1rpx solid rgba(37, 99, 235, 0.28);
}
</style>
